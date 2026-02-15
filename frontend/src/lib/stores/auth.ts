
import { writable } from 'svelte/store';
import { browser } from '$app/environment';

interface User {
    id: string;
    name: string;
    email: string;
    plan: string;
}

const initialUser: User | null = (browser && localStorage.getItem('auth_token'))
    ? JSON.parse(localStorage.getItem('auth_user') || 'null')
    : null;

export const auth = writable<User | null>(initialUser);

auth.subscribe((value) => {
    if (browser) {
        if (value) {
            localStorage.setItem('auth_user', JSON.stringify(value));
        } else {
            localStorage.removeItem('auth_user');
            localStorage.removeItem('auth_token');
        }
    }
});
