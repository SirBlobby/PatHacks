<script lang="ts">
    import { page } from "$app/stores";
    import { onMount } from "svelte";
    import {
        getDevice,
        updateDevice,
        deleteDevice,
        listDeviceRecordings,
        deleteRecording,
        type Recording,
    } from "$lib/api";
    import Icon from "@iconify/svelte";

    const deviceId = $page.params.id;

    let loading = $state(true);
    let error = $state("");
    let device = $state<any>(null);
    let recordings = $state<Recording[]>([]);
    let recordingsLoading = $state(true);
    let recordingsError = $state("");

    // Editing
    let editing = $state(false);
    let editName = $state("");
    let saving = $state(false);

    onMount(async () => {
        await fetchDevice();
        await fetchRecordings();
    });

    async function fetchDevice() {
        loading = true;
        error = "";
        try {
            device = await getDevice(deviceId);
        } catch (err: any) {
            error = err.message;
        } finally {
            loading = false;
        }
    }

    async function fetchRecordings() {
        recordingsLoading = true;
        recordingsError = "";
        try {
            recordings = await listDeviceRecordings(deviceId);
        } catch (err: any) {
            recordingsError = err.message;
        } finally {
            recordingsLoading = false;
        }
    }

    function startEdit() {
        editName = device?.name || "";
        editing = true;
    }

    async function saveEdit() {
        if (!editName.trim()) return;
        saving = true;
        try {
            await updateDevice(deviceId, { name: editName.trim() });
            device = { ...device, name: editName.trim() };
            editing = false;
        } catch (err: any) {
            alert(err.message);
        } finally {
            saving = false;
        }
    }

    async function handleDeleteDevice() {
        if (!confirm(`Delete device "${device?.name}"? This cannot be undone.`))
            return;
        try {
            await deleteDevice(deviceId);
            window.location.href = "/panel/devices";
        } catch (err: any) {
            alert(err.message);
        }
    }

    async function handleDeleteRecording(rec: Recording) {
        if (!confirm(`Delete recording "${rec.title}"?`)) return;
        try {
            await deleteRecording(rec.id);
            recordings = recordings.filter((r) => r.id !== rec.id);
        } catch (err: any) {
            alert(err.message);
        }
    }

    function timeAgo(dateStr: string | null): string {
        if (!dateStr) return "Never";
        const diff = Date.now() - new Date(dateStr).getTime();
        const mins = Math.floor(diff / 60000);
        if (mins < 1) return "Just now";
        if (mins < 60) return `${mins}m ago`;
        const hrs = Math.floor(mins / 60);
        if (hrs < 24) return `${hrs}h ago`;
        return `${Math.floor(hrs / 24)}d ago`;
    }

    function formatDuration(seconds: number): string {
        if (!seconds) return "0:00";
        const m = Math.floor(seconds / 60);
        const s = Math.floor(seconds % 60);
        return `${m}:${s.toString().padStart(2, "0")}`;
    }

    function statusColor(
        status: string,
    ): { bg: string; text: string; dot: string } {
        switch (status) {
            case "recording":
                return {
                    bg: "bg-red-500/10",
                    text: "text-red-400",
                    dot: "bg-red-500 animate-pulse",
                };
            case "processing":
            case "transcribing":
                return {
                    bg: "bg-yellow-500/10",
                    text: "text-yellow-400",
                    dot: "bg-yellow-500 animate-pulse",
                };
            case "completed":
                return {
                    bg: "bg-green-500/10",
                    text: "text-green-500",
                    dot: "bg-green-500",
                };
            case "error":
                return {
                    bg: "bg-red-500/10",
                    text: "text-red-500",
                    dot: "bg-red-500",
                };
            default:
                return {
                    bg: "bg-brand-text/5",
                    text: "text-brand-text/40",
                    dot: "bg-brand-text/30",
                };
        }
    }

    function formatDate(dateStr: string | null): string {
        if (!dateStr) return "--";
        return new Date(dateStr).toLocaleDateString("en-US", {
            month: "short",
            day: "numeric",
            hour: "2-digit",
            minute: "2-digit",
        });
    }
</script>

<div class="max-w-4xl mx-auto w-full">
    <!-- Back link -->
    <a
        href="/panel/devices"
        class="text-brand-text/50 hover:text-brand-text inline-flex items-center gap-1 mb-6 transition-colors"
    >
        <Icon icon="mdi:arrow-left" class="text-lg" />
        Back to Devices
    </a>

    {#if loading}
        <div class="flex justify-center py-20">
            <div
                class="w-10 h-10 border-4 border-brand-accent border-t-transparent rounded-full animate-spin"
            ></div>
        </div>
    {:else if error}
        <div
            class="p-4 bg-red-500/10 border border-red-500/30 rounded-xl text-red-500 text-center"
        >
            {error}
        </div>
    {:else if device}
        <!-- Device Header -->
        <div
            class="bg-brand-surface/80 backdrop-blur border border-brand-text/10 rounded-2xl p-6 mb-6"
        >
            <div class="flex items-start justify-between">
                <div class="flex items-center gap-4">
                    <div
                        class="w-14 h-14 rounded-xl bg-brand-accent/10 border border-brand-accent/20 flex items-center justify-center text-brand-accent"
                    >
                        <Icon icon="mdi:cellphone" class="text-3xl" />
                    </div>
                    <div>
                        {#if editing}
                            <div class="flex items-center gap-2">
                                <input
                                    type="text"
                                    bind:value={editName}
                                    class="bg-brand-bg border border-brand-text/20 rounded-lg px-3 py-1.5 text-brand-text font-bold text-xl focus:outline-none focus:border-brand-accent"
                                    onkeydown={(e) => e.key === "Enter" && saveEdit()}
                                />
                                <button
                                    onclick={saveEdit}
                                    disabled={saving}
                                    class="p-1.5 rounded-lg bg-brand-accent/20 text-brand-accent hover:bg-brand-accent/30 transition-colors"
                                >
                                    <Icon icon="mdi:check" class="text-lg" />
                                </button>
                                <button
                                    onclick={() => (editing = false)}
                                    class="p-1.5 rounded-lg bg-brand-text/5 text-brand-text/50 hover:text-brand-text transition-colors"
                                >
                                    <Icon icon="mdi:close" class="text-lg" />
                                </button>
                            </div>
                        {:else}
                            <div class="flex items-center gap-2">
                                <h1 class="text-2xl font-bold text-brand-text">
                                    {device.name}
                                </h1>
                                <button
                                    onclick={startEdit}
                                    class="p-1 rounded-lg text-brand-text/30 hover:text-brand-text/60 transition-colors"
                                    title="Rename device"
                                >
                                    <Icon
                                        icon="mdi:pencil-outline"
                                        class="text-sm"
                                    />
                                </button>
                            </div>
                        {/if}
                        <p
                            class="text-sm text-brand-text/50 font-mono tracking-wider mt-0.5"
                        >
                            KEY: {device.key}
                        </p>
                    </div>
                </div>

                <div class="flex items-center gap-3">
                    <span
                        class="flex items-center gap-1.5 text-xs px-2.5 py-1 rounded-full font-medium {device.status ===
                        'online'
                            ? 'bg-green-500/10 text-green-500'
                            : 'bg-brand-text/5 text-brand-text/40'}"
                    >
                        <span
                            class="w-2 h-2 rounded-full {device.status ===
                            'online'
                                ? 'bg-green-500'
                                : 'bg-brand-text/30'}"
                        ></span>
                        {device.status}
                    </span>
                    <button
                        onclick={handleDeleteDevice}
                        class="p-2 rounded-lg hover:bg-red-500/10 text-brand-text/30 hover:text-red-500 transition-colors"
                        title="Delete device"
                    >
                        <Icon icon="mdi:delete-outline" class="text-xl" />
                    </button>
                </div>
            </div>

            <!-- Device Info Row -->
            <div
                class="mt-5 pt-5 border-t border-brand-text/10 grid grid-cols-2 sm:grid-cols-3 gap-4"
            >
                <div>
                    <p class="text-xs text-brand-text/40 uppercase tracking-wider">
                        Last Seen
                    </p>
                    <p class="text-sm text-brand-text mt-0.5">
                        {timeAgo(device.last_seen)}
                    </p>
                </div>
                <div>
                    <p class="text-xs text-brand-text/40 uppercase tracking-wider">
                        Registered
                    </p>
                    <p class="text-sm text-brand-text mt-0.5">
                        {formatDate(device.created_at)}
                    </p>
                </div>
                <div>
                    <p class="text-xs text-brand-text/40 uppercase tracking-wider">
                        Recordings
                    </p>
                    <p class="text-sm text-brand-text mt-0.5">
                        {recordings.length}
                    </p>
                </div>
            </div>
        </div>

        <!-- Recordings Section -->
        <div>
            <h2 class="text-xl font-bold text-brand-text mb-4">Recordings</h2>

            {#if recordingsLoading}
                <div class="flex justify-center py-12">
                    <div
                        class="w-8 h-8 border-4 border-brand-accent border-t-transparent rounded-full animate-spin"
                    ></div>
                </div>
            {:else if recordingsError}
                <div
                    class="p-4 bg-red-500/10 border border-red-500/30 rounded-xl text-red-500 text-center"
                >
                    {recordingsError}
                </div>
            {:else if recordings.length === 0}
                <div
                    class="bg-brand-surface/50 border border-brand-text/10 rounded-2xl p-10 text-center"
                >
                    <Icon
                        icon="mdi:microphone-off"
                        class="text-4xl text-brand-text/20 mx-auto mb-3"
                    />
                    <p class="text-brand-text/50">
                        No recordings yet. Start recording from your device to
                        see them here.
                    </p>
                </div>
            {:else}
                <div class="space-y-3">
                    {#each recordings as rec}
                        {@const sc = statusColor(rec.status)}
                        <div
                            class="bg-brand-surface/80 backdrop-blur border border-brand-text/10 rounded-xl p-4 flex items-center justify-between hover:border-brand-text/20 transition-colors"
                        >
                            <div class="flex items-center gap-3 min-w-0">
                                <div
                                    class="w-10 h-10 rounded-lg {sc.bg} flex items-center justify-center flex-shrink-0"
                                >
                                    {#if rec.status === "recording"}
                                        <Icon
                                            icon="mdi:microphone"
                                            class="text-lg {sc.text}"
                                        />
                                    {:else if rec.status === "processing" || rec.status === "transcribing"}
                                        <Icon
                                            icon="mdi:cog"
                                            class="text-lg {sc.text} animate-spin"
                                        />
                                    {:else if rec.status === "completed"}
                                        <Icon
                                            icon="mdi:check-circle"
                                            class="text-lg {sc.text}"
                                        />
                                    {:else}
                                        <Icon
                                            icon="mdi:alert-circle"
                                            class="text-lg {sc.text}"
                                        />
                                    {/if}
                                </div>
                                <div class="min-w-0">
                                    <p
                                        class="font-medium text-brand-text truncate"
                                    >
                                        {rec.title}
                                    </p>
                                    <div
                                        class="flex items-center gap-3 text-xs text-brand-text/40 mt-0.5"
                                    >
                                        <span
                                            class="flex items-center gap-1 {sc.text}"
                                        >
                                            <span
                                                class="w-1.5 h-1.5 rounded-full {sc.dot}"
                                            ></span>
                                            {rec.status}
                                        </span>
                                        {#if rec.duration_seconds}
                                            <span class="flex items-center gap-1">
                                                <Icon
                                                    icon="mdi:clock-outline"
                                                    class="text-xs"
                                                />
                                                {formatDuration(rec.duration_seconds)}
                                            </span>
                                        {/if}
                                        <span>
                                            {formatDate(rec.started_at || rec.created_at)}
                                        </span>
                                    </div>
                                    {#if rec.status === "error" && rec.error}
                                        <p
                                            class="text-xs text-red-400 mt-1 truncate max-w-[300px]"
                                        >
                                            {rec.error}
                                        </p>
                                    {/if}
                                </div>
                            </div>

                            <div class="flex items-center gap-2 flex-shrink-0 ml-3">
                                {#if rec.status === "completed" && rec.source_id}
                                    <a
                                        href="/panel/sources/{rec.source_id}"
                                        class="px-3 py-1.5 text-xs font-medium bg-brand-accent/10 text-brand-accent rounded-lg hover:bg-brand-accent/20 transition-colors"
                                    >
                                        View Source
                                    </a>
                                {/if}
                                <button
                                    onclick={() => handleDeleteRecording(rec)}
                                    class="p-1.5 rounded-lg hover:bg-red-500/10 text-brand-text/30 hover:text-red-500 transition-colors"
                                    title="Delete recording"
                                >
                                    <Icon
                                        icon="mdi:delete-outline"
                                        class="text-lg"
                                    />
                                </button>
                            </div>
                        </div>
                    {/each}
                </div>
            {/if}
        </div>
    {/if}
</div>
