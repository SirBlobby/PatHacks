<script lang="ts">
    import { onMount } from "svelte";
    import { listDevices, deleteDevice } from "$lib/api";
    import Icon from "@iconify/svelte";

    let loading = $state(true);
    let error = $state("");
    let devices = $state([] as any[]);

    onMount(async () => {
        await fetchDevices();
    });

    async function fetchDevices() {
        loading = true;
        error = "";
        try {
            devices = await listDevices();
        } catch (err: any) {
            error = err.message;
        } finally {
            loading = false;
        }
    }

    async function handleDelete(id: string, name: string) {
        if (!confirm(`Delete device "${name}"? This cannot be undone.`)) return;
        try {
            await deleteDevice(id);
            devices = devices.filter((d: any) => d.id !== id);
        } catch (err: any) {
            alert(err.message);
        }
    }

    function timeAgo(dateStr: string | null): string {
        if (!dateStr) return "Never";
        const diff = Date.now() - new Date(dateStr).getTime();
        const mins = Math.floor(diff / 60000);
        if (mins < 60) return `${mins}m ago`;
        const hrs = Math.floor(mins / 60);
        if (hrs < 24) return `${hrs}h ago`;
        return `${Math.floor(hrs / 24)}d ago`;
    }
</script>

<div class="max-w-4xl mx-auto w-full">
    <div class="flex justify-between items-center mb-8">
        <h1 class="text-3xl font-bold text-brand-text">My Devices</h1>
        <a
            href="/panel/devices/new"
            class="px-4 py-2 bg-brand-accent text-black font-bold rounded-lg hover:scale-105 transition-transform shadow-lg shadow-brand-accent/20"
        >
            + Add Device
        </a>
    </div>

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
    {:else if devices.length === 0}
        <div
            class="bg-brand-surface/50 border border-brand-text/10 rounded-2xl p-12 text-center"
        >
            <p class="text-brand-text/50 text-lg mb-4">No devices paired yet</p>
            <a
                href="/panel/devices/new"
                class="inline-block px-6 py-3 bg-brand-accent text-black font-bold rounded-lg hover:scale-105 transition-transform"
            >
                Register Your First Device
            </a>
        </div>
    {:else}
        <div class="grid gap-4">
            {#each devices as device}
                <div
                    class="bg-brand-surface/80 backdrop-blur border border-brand-text/10 rounded-2xl p-6 flex items-center justify-between hover:border-brand-text/20 transition-colors"
                >
                    <div class="flex items-center gap-4">
                        <div
                            class="w-12 h-12 rounded-xl bg-brand-accent/10 border border-brand-accent/20 flex items-center justify-center text-brand-accent"
                        >
                            <Icon icon="mdi:cellphone" class="text-2xl" />
                        </div>
                        <div>
                            <p class="font-bold text-brand-text">
                                {device.name}
                            </p>
                            <p class="text-sm text-brand-text/50">
                                {device.device_type} • S/N: {device.serial_number}
                            </p>
                            <p class="text-xs text-brand-text/40 mt-0.5">
                                Last seen: {timeAgo(device.last_seen)}
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
                            onclick={() => handleDelete(device.id, device.name)}
                            class="p-2 rounded-lg hover:bg-red-500/10 text-brand-text/30 hover:text-red-500 transition-colors"
                            title="Delete device"
                        >
                            <Icon icon="mdi:delete-outline" class="text-lg" />
                        </button>
                    </div>
                </div>
            {/each}
        </div>
    {/if}
</div>
