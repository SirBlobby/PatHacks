<div class="max-w-md mx-auto py-24 px-4">
    <div
        class="bg-brand-bg/80 backdrop-blur-xl p-8 rounded-2xl border border-gray-800 shadow-2xl"
    >
        <h1 class="text-3xl font-bold mb-8 text-center text-brand-accent">
            Create Account
        </h1>

        <form class="space-y-6">
            <div>
                <label
                    for="name"
                    class="block text-sm font-medium text-gray-400 mb-2"
                    >Full Name</label
                >
                <input
                    type="text"
                    id="name"
                    class="w-full p-3 bg-black/50 border border-gray-700 rounded-lg focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all placeholder-gray-600"
                    placeholder="John Doe"
                />
            </div>

            <div>
                <label
                    for="email"
                    class="block text-sm font-medium text-gray-400 mb-2"
                    >Email Address</label
                >
                <input
                    type="email"
                    id="email"
                    class="w-full p-3 bg-black/50 border border-gray-700 rounded-lg focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all placeholder-gray-600"
                    placeholder="you@example.com"
                />
            </div>

            <div>
                <label
                    for="password"
                    class="block text-sm font-medium text-gray-400 mb-2"
                    >Password</label
                >
                <input
                    type="password"
                    id="password"
                    class="w-full p-3 bg-black/50 border border-gray-700 rounded-lg focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all placeholder-gray-600"
                    placeholder="••••••••"
                />
            </div>

            <button
                type="submit"
                class="w-full bg-brand-accent text-black font-bold py-3 rounded-lg hover:opacity-90 hover:scale-[1.02] transition-all shadow-lg shadow-brand-accent/20"
            >
                Register
            </button>
        </form>

        <p class="text-center mt-6 text-gray-400 text-sm">
            Already have an account? <a
                href="/signin"
                class="text-brand-accent hover:underline font-medium">Sign in</a
            >
        </p>
    </div>
</div>
