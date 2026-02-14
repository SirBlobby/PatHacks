<h1 class="text-3xl font-bold text-white mb-8">Your Profile</h1>
<div
    class="max-w-2xl bg-gray-900 border border-gray-800 p-10 rounded-xl shadow-lg relative overflow-hidden"
>
    <div class="absolute top-0 left-0 w-full h-32 bg-brand-accent/10 z-0"></div>

    <div class="relative z-10 flex flex-col items-center text-center -mt-10">
        <div
            class="w-32 h-32 rounded-full border-4 border-gray-900 bg-brand-accent shadow-xl mb-4"
        ></div>
        <h2 class="text-2xl font-bold text-white mb-1">John Doe</h2>
        <p class="text-gray-400 mb-6">Student • University of Tech</p>

        <div class="flex gap-4 mb-8">
            <div
                class="bg-black/30 px-6 py-2 rounded-lg border border-gray-800"
            >
                <div class="text-xl font-bold text-white">24</div>
                <div class="text-xs text-gray-500 uppercase tracking-wider">
                    Recordings
                </div>
            </div>
            <div
                class="bg-black/30 px-6 py-2 rounded-lg border border-gray-800"
            >
                <div class="text-xl font-bold text-white">Pro</div>
                <div class="text-xs text-gray-500 uppercase tracking-wider">
                    Plan
                </div>
            </div>
        </div>

        <div class="w-full space-y-4 text-left">
            <div>
                <label
                    for="display-name"
                    class="block text-gray-500 text-sm font-bold mb-1 ml-1"
                    >Display Name</label
                >
                <input
                    id="display-name"
                    type="text"
                    value="John Doe"
                    class="w-full bg-black/50 border border-gray-700 rounded-lg p-3 text-white focus:border-brand-accent outline-none"
                />
            </div>
            <div>
                <label
                    for="email"
                    class="block text-gray-500 text-sm font-bold mb-1 ml-1"
                    >Email</label
                >
                <input
                    id="email"
                    type="email"
                    value="john.doe@example.com"
                    class="w-full bg-black/50 border border-gray-700 rounded-lg p-3 text-white focus:border-brand-accent outline-none"
                />
            </div>
            <button
                class="w-full bg-brand-accent text-black font-bold py-3 rounded-lg hover:opacity-90 transition shadow-lg shadow-brand-accent/20 mt-4"
                >Save Changes</button
            >
        </div>
    </div>
</div>
