<div class="max-w-md mx-auto py-24 px-4">
    <div
        class="bg-brand-surface/80 backdrop-blur-xl p-8 rounded-2xl border border-brand-text/10 shadow-2xl"
    >
        <h1 class="text-3xl font-bold mb-8 text-center text-brand-accent">
            Create Account
        </h1>

        <form class="space-y-6">
            <div>
                <label
                    for="name"
                    class="block text-sm font-bold text-brand-text/70 mb-2 ml-1"
                    >Full Name</label
                >
                <input
                    type="text"
                    id="name"
                    class="w-full p-3 bg-brand-bg border border-brand-text/20 rounded-lg text-brand-text focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all placeholder-brand-text/30 shadow-inner"
                    placeholder="John Doe"
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
                    class="w-full p-3 bg-brand-bg border border-brand-text/20 rounded-lg text-brand-text focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all placeholder-brand-text/30 shadow-inner"
                    placeholder="you@example.com"
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
                    class="w-full p-3 bg-brand-bg border border-brand-text/20 rounded-lg text-brand-text focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all placeholder-brand-text/30 shadow-inner"
                    placeholder="••••••••"
                />
            </div>

            <button
                type="submit"
                class="w-full bg-brand-accent text-black font-bold py-3 rounded-lg hover:scale-[1.02] active:scale-[0.98] transition-all shadow-lg shadow-brand-accent/20 cursor-pointer"
            >
                Register
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
