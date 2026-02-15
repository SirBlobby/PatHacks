/**
 * LearningBuddy - Centralized API Client.
 * All backend requests go through this module.
 */

import { browser } from '$app/environment';
import { auth } from './stores/auth';

const API_BASE = '/api';

function getToken(): string | null {
    if (!browser) return null;
    return localStorage.getItem('auth_token');
}

async function request<T = any>(
    endpoint: string,
    options: RequestInit = {}
): Promise<T> {
    const token = getToken();
    const headers: Record<string, string> = {
        'Content-Type': 'application/json',
        ...(options.headers as Record<string, string> || {}),
    };

    if (token) {
        headers['Authorization'] = `Bearer ${token}`;
    }

    const res = await fetch(`${API_BASE}${endpoint}`, {
        ...options,
        headers,
    });

    if (res.status === 401) {
        if (browser) {
            auth.set(null);
            window.location.href = '/signin';
        }
        throw new Error('Unauthorized');
    }

    const contentType = res.headers.get('content-type') || '';
    if (!contentType.includes('application/json')) {
        if (!res.ok) {
            throw new Error(`Server error (${res.status}). Check that MongoDB is configured in backend/.env`);
        }
        throw new Error('Server returned an unexpected response. Is the backend running?');
    }

    const data = await res.json();

    if (!res.ok) {
        throw new Error(data.error || `Request failed (${res.status})`);
    }

    return data as T;
}

/**
 * Upload a file via multipart form data (no JSON Content-Type header).
 */
async function uploadRequest<T = any>(
    endpoint: string,
    formData: FormData
): Promise<T> {
    const token = getToken();
    const headers: Record<string, string> = {};
    if (token) {
        headers['Authorization'] = `Bearer ${token}`;
    }

    const res = await fetch(`${API_BASE}${endpoint}`, {
        method: 'POST',
        headers,
        body: formData,
    });

    if (res.status === 401) {
        if (browser) {
            auth.set(null);
            window.location.href = '/signin';
        }
        throw new Error('Unauthorized');
    }

    const contentType = res.headers.get('content-type') || '';
    if (!contentType.includes('application/json')) {
        if (!res.ok) {
            throw new Error(`Server error (${res.status})`);
        }
        throw new Error('Server returned an unexpected response.');
    }

    const data = await res.json();
    if (!res.ok) {
        throw new Error(data.error || `Upload failed (${res.status})`);
    }
    return data as T;
}

// ── Auth ──

export async function register(name: string, email: string, password: string) {
    const data = await request('/auth/register', {
        method: 'POST',
        body: JSON.stringify({ name, email, password }),
    });
    if (browser && data.access_token) {
        localStorage.setItem('auth_token', data.access_token);
        auth.set(data.user);
    }
    return data;
}

export async function login(email: string, password: string) {
    const data = await request('/auth/login', {
        method: 'POST',
        body: JSON.stringify({ email, password }),
    });
    if (browser && data.access_token) {
        localStorage.setItem('auth_token', data.access_token);
        auth.set(data.user);
    }
    return data;
}

export async function getMe() {
    return request('/auth/me');
}

export function logout() {
    if (browser) {
        auth.set(null);
    }
}

// ── Dashboard ──

export async function getDashboard() {
    return request('/dashboard');
}

// ── Devices ──

export async function listDevices() {
    return request('/devices');
}

export async function getDevice(id: string) {
    return request(`/devices/${id}`);
}

export async function registerDevice(data: { serial_number: string; name: string; device_type: string }) {
    return request('/devices', {
        method: 'POST',
        body: JSON.stringify(data),
    });
}

export async function updateDevice(id: string, data: { name?: string; device_type?: string }) {
    return request(`/devices/${id}`, {
        method: 'PUT',
        body: JSON.stringify(data),
    });
}

export async function deleteDevice(id: string) {
    return request(`/devices/${id}`, { method: 'DELETE' });
}

// ── Sources ──

export async function listSources(query?: string) {
    const q = query ? `?q=${encodeURIComponent(query)}` : '';
    return request(`/sources${q}`);
}

export async function getSource(id: string) {
    return request(`/sources/${id}`);
}

export async function createSourceText(data: { title: string; content: string }) {
    return request('/sources', {
        method: 'POST',
        body: JSON.stringify(data),
    });
}

export async function uploadSourceFile(file: File, title?: string) {
    const formData = new FormData();
    formData.append('file', file);
    if (title) {
        formData.append('title', title);
    }
    return uploadRequest('/sources', formData);
}

export async function deleteSource(id: string) {
    return request(`/sources/${id}`, { method: 'DELETE' });
}

export async function regenerateSummary(id: string) {
    return request(`/sources/${id}/regenerate-summary`, { method: 'POST' });
}

// ── Chat ──

export async function sendChatMessage(sourceId: string, message: string) {
    return request(`/sources/${sourceId}/chat`, {
        method: 'POST',
        body: JSON.stringify({ message }),
    });
}

export async function getChatHistory(sourceId: string) {
    return request(`/sources/${sourceId}/chat/history`);
}

export async function clearChatHistory(sourceId: string) {
    return request(`/sources/${sourceId}/chat/history`, { method: 'DELETE' });
}

/**
 * Stream a chat response via SSE.
 */
export async function streamChatMessage(
    sourceId: string,
    message: string,
    onChunk: (chunk: string) => void
): Promise<string> {
    const token = getToken();
    const res = await fetch(`${API_BASE}/sources/${sourceId}/chat/stream`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            ...(token ? { Authorization: `Bearer ${token}` } : {}),
        },
        body: JSON.stringify({ message }),
    });

    if (!res.ok) {
        const err = await res.json();
        throw new Error(err.error || 'Stream failed');
    }

    const reader = res.body?.getReader();
    const decoder = new TextDecoder();
    let fullResponse = '';

    if (reader) {
        while (true) {
            const { done, value } = await reader.read();
            if (done) break;

            const text = decoder.decode(value, { stream: true });
            const lines = text.split('\n');
            for (const line of lines) {
                if (line.startsWith('data: ')) {
                    try {
                        const parsed = JSON.parse(line.slice(6));
                        if (parsed.chunk) {
                            fullResponse += parsed.chunk;
                            onChunk(parsed.chunk);
                        }
                        if (parsed.error) {
                            throw new Error(parsed.error);
                        }
                    } catch (e) {
                        // ignore parse errors on incomplete chunks
                    }
                }
            }
        }
    }

    return fullResponse;
}

// ── Profile ──

export async function getProfile() {
    return request('/profile');
}

export async function updateProfile(data: { name?: string; email?: string }) {
    return request('/profile', {
        method: 'PUT',
        body: JSON.stringify(data),
    });
}

// ── Settings ──

export async function getSettings() {
    return request('/settings');
}

export async function updateSettings(data: { theme?: string }) {
    return request('/settings', {
        method: 'PUT',
        body: JSON.stringify(data),
    });
}

export async function changePassword(currentPassword: string, newPassword: string) {
    return request('/settings/change-password', {
        method: 'POST',
        body: JSON.stringify({ current_password: currentPassword, new_password: newPassword }),
    });
}

export async function deleteAccount() {
    return request('/settings/delete-account', { method: 'DELETE' });
}

// ── Utility ──

export function isAuthenticated(): boolean {
    if (!browser) return false;
    return !!localStorage.getItem('auth_token');
}

export function getCurrentUser(): { id: string; name: string; email: string; plan: string } | null {
    if (!browser) return null;
    const raw = localStorage.getItem('auth_user');
    if (!raw) return null;
    try {
        return JSON.parse(raw);
    } catch {
        return null;
    }
}
