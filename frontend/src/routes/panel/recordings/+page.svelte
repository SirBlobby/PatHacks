<div class="max-w-6xl mx-auto w-full">
    <div
        class="flex flex-col md:flex-row justify-between items-center mb-8 gap-4"
    >
        <h1 class="text-3xl font-bold text-brand-text">Recordings</h1>
        <div class="relative w-full md:w-auto">
            <input
                type="text"
                placeholder="Search..."
                class="w-full md:w-64 bg-brand-surface border border-brand-text/20 rounded-lg py-2 px-4 text-brand-text placeholder-brand-text/50 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-sm"
            />
        </div>
    </div>

    <div class="space-y-4">
        {#each [1, 2, 3, 4] as i}
            <div
                class="bg-brand-surface backdrop-blur border-2 border-brand-text/10 p-6 rounded-xl hover:border-brand-accent hover:shadow-lg hover:shadow-brand-accent/20 transition-all duration-300 cursor-pointer group"
            >
                <div
                    class="flex flex-col md:flex-row justify-between md:items-center gap-4"
                >
                    <div class="flex items-start gap-4">
                        <div
                            class="w-12 h-12 rounded-full bg-brand-accent/10 flex items-center justify-center text-brand-accent text-xl shrink-0"
                        >
                            <span class="font-bold">#</span>
                        </div>
                        <div>
                            <h3
                                class="text-xl font-bold text-brand-text group-hover:text-brand-accent transition-colors"
                            >
                                Lecture #{i}: Advanced Topics
                            </h3>
                            <div
                                class="flex items-center gap-4 text-sm text-brand-text/70 mt-1"
                            >
                                <span>Oct 24, 2023</span>
                                <span>1h 24m</span>
                                <span>45 MB</span>
                            </div>
                        </div>
                    </div>

                    <div class="flex items-center gap-2 flex-wrap">
                        <span
                            class="px-3 py-1 rounded-full bg-green-500/10 text-green-600 border border-green-500/20 text-xs font-bold uppercase"
                            >Processed</span
                        >
                        <div class="flex gap-2 ml-auto md:ml-4">
                            <a
                                href="/panel/recordings/{i}"
                                class="bg-brand-text/5 hover:bg-brand-text/10 text-brand-text px-4 py-2 rounded-lg text-sm font-medium transition"
                                >View</a
                            >
                            <a
                                href="/panel/recordings/{i}/chat"
                                class="bg-brand-accent/10 hover:bg-brand-accent/20 text-brand-accent border border-brand-accent/20 px-4 py-2 rounded-lg text-sm font-medium transition flex items-center gap-2"
                                >Chat</a
                            >
                        </div>
                    </div>
                </div>
            </div>
        {/each}
    </div>
</div>
