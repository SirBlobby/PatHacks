<script lang="ts">
    import { goto } from "$app/navigation";
    import { registerDevice } from "$lib/api";
    import Icon from "@iconify/svelte";

    let deviceKey = $state("");
    let deviceName = $state("");
    let error = $state("");
    let loading = $state(false);

    async function handleSubmit(e: Event) {
        e.preventDefault();
        error = "";
        loading = true;

        try {
            await registerDevice({
                key: deviceKey,
                name: deviceName,
            });
            goto("/panel/devices");
        } catch (err: any) {
            error = err.message || "Failed to register device";
        } finally {
            loading = false;
        }
    }
</script>

<div class="max-w-2xl mx-auto w-full">
    <div class="flex items-center gap-4 mb-8">
        <a
            href="/panel/devices"
            class="text-brand-text/50 hover:text-brand-text transition flex items-center gap-1 group"
        >
            <Icon
                icon="mdi:arrow-left"
                class="group-hover:-translate-x-1 transition-transform"
            />
            Back</a
        >
        <h1 class="text-3xl font-bold text-brand-text">Register New Device</h1>
    </div>

    <div
        class="bg-brand-surface backdrop-blur border-2 border-brand-text/10 p-8 rounded-xl shadow-lg"
    >
        {#if error}
            <div
                class="mb-6 p-3 bg-red-500/10 border border-red-500/30 rounded-lg text-red-500 text-sm text-center"
            >
                {error}
            </div>
        {/if}

        <form class="space-y-6" onsubmit={handleSubmit}>
            <div>
                <label
                    for="device-key"
                    class="block text-brand-text/70 text-sm font-bold mb-2 ml-1"
                    >Device Key</label
                >
                <input
                    id="device-key"
                    type="text"
                    bind:value={deviceKey}
                    class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner font-mono tracking-widest uppercase"
                    placeholder="ENTER KEY DISPLAYED ON SCREEN"
                    required
                />
            </div>

            <div>
                <label
                    for="device-name"
                    class="block text-brand-text/70 text-sm font-bold mb-2 ml-1"
                    >Device Name</label
                >
                <input
                    id="device-name"
                    type="text"
                    bind:value={deviceName}
                    class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner"
                    placeholder="e.g. My Learning Buddy"
                    required
                />
            </div>

            <div class="pt-4">
                <button
                    type="submit"
                    disabled={loading}
                    class="w-full bg-brand-accent text-black font-bold py-3 rounded-lg hover:scale-[1.02] active:scale-[0.98] transition-transform shadow-lg shadow-brand-accent/20 cursor-pointer disabled:opacity-50 disabled:cursor-not-allowed"
                >
                    {loading ? "Pairing..." : "Pair Device"}
                </button>
            </div>
        </form>
    </div>
</div>
