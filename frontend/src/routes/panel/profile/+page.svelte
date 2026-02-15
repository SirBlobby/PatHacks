<script lang="ts">
    import { onMount } from "svelte";
    import { getProfile, updateProfile } from "$lib/api";

    let loading = $state(true);
    let saving = $state(false);
    let error = $state("");
    let success = $state("");
    let name = $state("");
    let email = $state("");
    let plan = $state("free");
    let sourceCount = $state(0);
    let createdAt = $state("");

    onMount(async () => {
        try {
            const profile = await getProfile();
            name = profile.name;
            email = profile.email;
            plan = profile.plan;
            sourceCount = profile.source_count;
            createdAt = profile.created_at
                ? new Date(profile.created_at).toLocaleDateString("en-US", {
                      month: "long",
                      day: "numeric",
                      year: "numeric",
                  })
                : "";
        } catch (err: any) {
            error = err.message;
        } finally {
            loading = false;
        }
    });

    async function handleSave(e: Event) {
        e.preventDefault();
        saving = true;
        error = "";
        success = "";

        try {
            await updateProfile({ name, email });
            // Update local storage user too
            const raw = localStorage.getItem("auth_user");
            if (raw) {
                const user = JSON.parse(raw);
                user.name = name;
                user.email = email;
                localStorage.setItem("auth_user", JSON.stringify(user));
            }
            success = "Profile updated successfully!";
        } catch (err: any) {
            error = err.message;
        } finally {
            saving = false;
        }
    }
</script>

<div class="max-w-2xl mx-auto w-full">
    <h1 class="text-3xl font-bold text-brand-text mb-8">Profile</h1>

    {#if loading}
        <div class="flex justify-center py-20">
            <div
                class="w-10 h-10 border-4 border-brand-accent border-t-transparent rounded-full animate-spin"
            ></div>
        </div>
    {:else}
        <!-- Profile Stats -->
        <div
            class="grid grid-cols-3 gap-4 mb-8 bg-brand-surface/60 border border-brand-text/10 rounded-2xl p-6"
        >
            <div class="text-center">
                <p class="text-2xl font-bold text-brand-text">
                    {sourceCount}
                </p>
                <p class="text-xs text-brand-text/50">Sources</p>
            </div>
            <div class="text-center">
                <p
                    class="text-2xl font-bold {plan === 'pro'
                        ? 'text-brand-accent'
                        : 'text-brand-text'}"
                >
                    {plan.charAt(0).toUpperCase() + plan.slice(1)}
                </p>
                <p class="text-xs text-brand-text/50">Plan</p>
            </div>
            <div class="text-center">
                <p class="text-2xl font-bold text-brand-text">
                    {createdAt || "—"}
                </p>
                <p class="text-xs text-brand-text/50">Member Since</p>
            </div>
        </div>

        {#if success}
            <div
                class="mb-6 p-3 bg-green-500/10 border border-green-500/30 rounded-lg text-green-500 text-sm text-center"
            >
                {success}
            </div>
        {/if}
        {#if error}
            <div
                class="mb-6 p-3 bg-red-500/10 border border-red-500/30 rounded-lg text-red-500 text-sm text-center"
            >
                {error}
            </div>
        {/if}

        <div
            class="bg-brand-surface backdrop-blur border-2 border-brand-text/10 p-8 rounded-xl shadow-lg"
        >
            <form class="space-y-6" onsubmit={handleSave}>
                <div>
                    <label
                        for="display-name"
                        class="block text-brand-text/70 text-sm font-bold mb-2 ml-1"
                        >Display Name</label
                    >
                    <input
                        id="display-name"
                        type="text"
                        bind:value={name}
                        class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner"
                        required
                    />
                </div>

                <div>
                    <label
                        for="email"
                        class="block text-brand-text/70 text-sm font-bold mb-2 ml-1"
                        >Email</label
                    >
                    <input
                        id="email"
                        type="email"
                        bind:value={email}
                        class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner"
                        required
                    />
                </div>

                <button
                    type="submit"
                    disabled={saving}
                    class="w-full bg-brand-accent text-black font-bold py-3 rounded-lg hover:scale-[1.02] active:scale-[0.98] transition-transform shadow-lg shadow-brand-accent/20 cursor-pointer disabled:opacity-50 disabled:cursor-not-allowed"
                >
                    {saving ? "Saving..." : "Save Changes"}
                </button>
            </form>
        </div>
    {/if}
</div>
