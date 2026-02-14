<div class="flex justify-between items-center mb-8">
    <h1 class="text-3xl font-bold text-white">Recordings</h1>
    <div class="relative">
        <input
            type="text"
            placeholder="Search..."
            class="bg-black/50 border border-gray-700 rounded-lg py-2 px-4 pl-4 text-white focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none"
        />
    </div>
</div>

<div class="space-y-4">
    {#each [1, 2, 3, 4] as i}
        <div
            class="bg-gray-900 border border-gray-800 p-6 rounded-xl hover:bg-gray-800/50 hover:border-brand-accent transition cursor-pointer group"
        >
            <div
                class="flex flex-col md:flex-row justify-between md:items-center gap-4"
            >
                <div class="flex items-start gap-4">
                    <div
                        class="w-12 h-12 rounded-full bg-brand-accent/10 flex items-center justify-center text-brand-accent text-xl"
                    >
                        <span class="font-bold">#</span>
                    </div>
                    <div>
                        <h3
                            class="text-xl font-bold text-white group-hover:text-brand-accent transition-colors"
                        >
                            Lecture #{i}: Advanced Topics
                        </h3>
                        <div
                            class="flex items-center gap-4 text-sm text-gray-400 mt-1"
                        >
                            <span>Oct 24, 2023</span>
                            <span>1h 24m</span>
                            <span>45 MB</span>
                        </div>
                    </div>
                </div>

                <div class="flex items-center gap-2">
                    <span
                        class="px-3 py-1 rounded-full bg-green-500/20 text-green-500 text-xs font-bold uppercase"
                        >Processed</span
                    >
                    <div class="flex gap-2 ml-4">
                        <a
                            href="/panel/recordings/{i}"
                            class="bg-white/5 hover:bg-white/10 text-white px-4 py-2 rounded-lg text-sm font-medium transition"
                            >View</a
                        >
                        <a
                            href="/panel/recordings/{i}/chat"
                            class="bg-brand-accent/20 hover:bg-brand-accent/30 text-brand-accent px-4 py-2 rounded-lg text-sm font-medium transition flex items-center gap-2"
                            >Chat</a
                        >
                    </div>
                </div>
            </div>
        </div>
    {/each}
</div>
