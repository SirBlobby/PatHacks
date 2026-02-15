<script lang="ts">
    import { goto } from "$app/navigation";
    import { register } from "$lib/api";

    let name = $state("");
    let email = $state("");
    let password = $state("");
    let error = $state("");
    let loading = $state(false);

    async function handleSubmit(e: Event) {
        e.preventDefault();
        error = "";
        loading = true;

        try {
            await register(name, email, password);
            goto("/panel/dashboard");
        } catch (err: any) {
            error = err.message || "Registration failed";
        } finally {
            loading = false;
        }
    }
</script>

<div class="max-w-md mx-auto py-24 px-4">
    <div
        class="bg-brand-surface/80 backdrop-blur-xl p-8 rounded-2xl border border-brand-text/10 shadow-2xl"
    >
        <h1 class="text-3xl font-bold mb-8 text-center text-brand-accent">
            Create Account
        </h1>

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
                    for="name"
                    class="block text-sm font-bold text-brand-text/70 mb-2 ml-1"
                    >Full Name</label
                >
                <input
                    type="text"
                    id="name"
                    bind:value={name}
                    class="w-full p-3 bg-brand-bg border border-brand-text/20 rounded-lg text-brand-text focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all placeholder-brand-text/30 shadow-inner"
                    placeholder="John Doe"
                    required
                />
            </div>

            <div>
                <label
                    for="email"
                    class="block text-sm font-bold text-brand-text/70 mb-2 ml-1"
                    >Email Address</label
                >
                <input
                    type="email"
                    id="email"
                    bind:value={email}
                    class="w-full p-3 bg-brand-bg border border-brand-text/20 rounded-lg text-brand-text focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all placeholder-brand-text/30 shadow-inner"
                    placeholder="you@example.com"
                    required
                />
            </div>

            <div>
                <label
                    for="password"
                    class="block text-sm font-bold text-brand-text/70 mb-2 ml-1"
                    >Password</label
                >
                <input
                    type="password"
                    id="password"
                    bind:value={password}
                    class="w-full p-3 bg-brand-bg border border-brand-text/20 rounded-lg text-brand-text focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all placeholder-brand-text/30 shadow-inner"
                    placeholder="••••••••"
                    required
                    minlength="6"
                />
            </div>

            <button
                type="submit"
                disabled={loading}
                class="w-full bg-brand-accent text-black font-bold py-3 rounded-lg hover:scale-[1.02] active:scale-[0.98] transition-all shadow-lg shadow-brand-accent/20 cursor-pointer disabled:opacity-50 disabled:cursor-not-allowed"
            >
                {loading ? "Creating Account..." : "Register"}
            </button>
        </form>

        <p class="text-center mt-6 text-brand-text/60 text-sm">
            Already have an account? <a
                href="/signin"
                class="text-brand-accent hover:underline font-bold">Sign in</a
            >
        </p>
    </div>
</div>
