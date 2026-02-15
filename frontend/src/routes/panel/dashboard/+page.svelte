<script lang="ts">
    import { onMount } from "svelte";
    import { getDashboard } from "$lib/api";
    import Icon from "@iconify/svelte";

    let loading = $state(true);
    let error = $state("");
    let stats = $state({
        total_sources: 0,
        hours_processed: 0,
        active_devices: 0,
        online_devices: 0,
        recent_activity: [] as any[],
    });

    onMount(async () => {
        try {
            stats = await getDashboard();
        } catch (err: any) {
            error = err.message;
        } finally {
            loading = false;
        }
    });

    function formatDuration(seconds: number): string {
        const mins = Math.floor(seconds / 60);
        if (mins < 60) return `${mins}m`;
        const hrs = Math.floor(mins / 60);
        const remMins = mins % 60;
        return `${hrs}h ${remMins}m`;
    }

    function timeAgo(dateStr: string | null): string {
        if (!dateStr) return "—";
        const diff = Date.now() - new Date(dateStr).getTime();
        const mins = Math.floor(diff / 60000);
        if (mins < 60) return `${mins}m ago`;
        const hrs = Math.floor(mins / 60);
        if (hrs < 24) return `${hrs}h ago`;
        const days = Math.floor(hrs / 24);
        return `${days}d ago`;
    }
</script>

<div class="max-w-6xl mx-auto w-full">
    <h1 class="text-3xl font-bold text-brand-text mb-8">Dashboard</h1>

    {#if loading}
        <div class="flex justify-center py-20">
            <div
                class="w-10 h-10 border-4 border-brand-accent border-t-transparent rounded-full animate-spin"
            ></div>
        </div>
    {:else if error}
        <div
            class="p-4 bg-red-500/10 border border-red-500/30 rounded-xl text-red-500 text-center flex flex-col items-center gap-2"
        >
            <Icon icon="mdi:alert-circle-outline" class="text-3xl" />
            {error}
        </div>
    {:else}
        <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-6 mb-10">
            <div
                class="bg-brand-surface/80 backdrop-blur border border-brand-text/10 p-6 rounded-2xl shadow-lg hover:shadow-xl transition-shadow relative overflow-hidden group"
            >
                <div
                    class="absolute -right-4 -top-4 text-brand-text/5 transform rotate-12 group-hover:scale-110 transition-transform"
                >
                    <Icon
                        icon="mdi:file-document-multiple-outline"
                        class="text-9xl"
                    />
                </div>
                <p class="text-brand-text/50 text-sm font-medium relative z-10">
                    Total Sources
                </p>
                <p
                    class="text-3xl font-bold text-brand-text mt-2 relative z-10"
                >
                    {stats.total_sources}
                </p>
            </div>
            <div
                class="bg-brand-surface/80 backdrop-blur border border-brand-text/10 p-6 rounded-2xl shadow-lg hover:shadow-xl transition-shadow relative overflow-hidden group"
            >
                <div
                    class="absolute -right-4 -top-4 text-brand-text/5 transform rotate-12 group-hover:scale-110 transition-transform"
                >
                    <Icon icon="mdi:clock-time-four-outline" class="text-9xl" />
                </div>
                <p class="text-brand-text/50 text-sm font-medium relative z-10">
                    Hours Processed
                </p>
                <p
                    class="text-3xl font-bold text-brand-accent mt-2 relative z-10"
                >
                    {stats.hours_processed}h
                </p>
            </div>
            <div
                class="bg-brand-surface/80 backdrop-blur border border-brand-text/10 p-6 rounded-2xl shadow-lg hover:shadow-xl transition-shadow relative overflow-hidden group"
            >
                <div
                    class="absolute -right-4 -top-4 text-brand-text/5 transform rotate-12 group-hover:scale-110 transition-transform"
                >
                    <Icon icon="mdi:devices" class="text-9xl" />
                </div>
                <p class="text-brand-text/50 text-sm font-medium relative z-10">
                    Active Devices
                </p>
                <p
                    class="text-3xl font-bold text-brand-text mt-2 relative z-10"
                >
                    {stats.active_devices}
                </p>
            </div>
            <div
                class="bg-brand-surface/80 backdrop-blur border border-brand-text/10 p-6 rounded-2xl shadow-lg hover:shadow-xl transition-shadow relative overflow-hidden group"
            >
                <div
                    class="absolute -right-4 -top-4 text-brand-text/5 transform rotate-12 group-hover:scale-110 transition-transform"
                >
                    <Icon icon="mdi:wifi" class="text-9xl" />
                </div>
                <p class="text-brand-text/50 text-sm font-medium relative z-10">
                    Devices Online
                </p>
                <p class="text-3xl font-bold mt-2 relative z-10">
                    <span class="text-green-500">{stats.online_devices}</span>
                    <span class="text-brand-text/30 text-lg"
                        >/ {stats.active_devices}</span
                    >
                </p>
            </div>
        </div>

        <div>
            <h2 class="text-xl font-bold text-brand-text mb-4">
                Recent Activity
            </h2>
            {#if stats.recent_activity.length === 0}
                <div
                    class="bg-brand-surface/50 border border-brand-text/10 rounded-2xl p-8 text-center flex flex-col items-center"
                >
                    <Icon
                        icon="mdi:bookshelf"
                        class="text-6xl text-brand-text/20 mb-4"
                    />
                    <p class="text-brand-text/50">
                        No sources yet. Upload a file or paste text to get
                        started!
                    </p>
                    <a
                        href="/panel/sources/new"
                        class="inline-flex items-center gap-2 mt-4 px-6 py-2 bg-brand-accent text-black font-bold rounded-lg hover:scale-105 transition-transform"
                    >
                        <Icon icon="mdi:plus-circle-outline" class="text-xl" />
                        Add Source
                    </a>
                </div>
            {:else}
                <div class="space-y-3">
                    {#each stats.recent_activity as rec}
                        <a
                            href="/panel/sources/{rec.id}"
                            class="flex items-center justify-between bg-brand-surface/60 border border-brand-text/10 rounded-xl p-4 hover:bg-brand-surface/80 transition-colors group"
                        >
                            <div class="flex items-center gap-4">
                                <div
                                    class="w-10 h-10 rounded-lg bg-brand-text/5 flex items-center justify-center text-brand-accent"
                                >
                                    <Icon
                                        icon={rec.file_type === "audio"
                                            ? "mdi:headphones"
                                            : rec.file_type === "video"
                                              ? "mdi:filmstrip"
                                              : "mdi:file-document-outline"}
                                        class="text-2xl"
                                    />
                                </div>
                                <div>
                                    <p
                                        class="font-medium text-brand-text group-hover:text-brand-accent transition-colors"
                                    >
                                        {rec.title}
                                    </p>
                                    <p
                                        class="text-sm text-brand-text/50 flex items-center gap-2"
                                    >
                                        {#if rec.duration_seconds}
                                            <span
                                                >{formatDuration(
                                                    rec.duration_seconds,
                                                )}</span
                                            >
                                            <span
                                                class="w-1 h-1 rounded-full bg-brand-text/30"
                                            ></span>
                                        {/if}
                                        <span>{timeAgo(rec.created_at)}</span>
                                    </p>
                                </div>
                            </div>
                            <div class="flex items-center gap-3">
                                <span
                                    class="text-xs px-2 py-1 rounded-full font-medium flex items-center gap-1 {rec.status ===
                                    'processed'
                                        ? 'bg-green-500/10 text-green-500'
                                        : rec.status === 'error'
                                          ? 'bg-red-500/10 text-red-500'
                                          : 'bg-yellow-500/10 text-yellow-500'}"
                                >
                                    {#if rec.status === "processed"}
                                        <Icon icon="mdi:check-circle-outline" />
                                    {:else if rec.status === "error"}
                                        <Icon icon="mdi:alert-circle-outline" />
                                    {:else}
                                        <Icon icon="mdi:clock-outline" />
                                    {/if}
                                    {rec.status}
                                </span>
                                <Icon
                                    icon="mdi:chevron-right"
                                    class="text-brand-text/30 group-hover:text-brand-accent transition-colors text-xl"
                                />
                            </div>
                        </a>
                    {/each}
                </div>
            {/if}
        </div>
    {/if}
</div>
