<div class="max-w-6xl mx-auto w-full">
    <div class="flex justify-between items-center mb-8">
        <h1 class="text-3xl font-bold text-brand-text">Your Devices</h1>
        <a
            href="/panel/devices/new"
            class="bg-brand-accent text-black px-4 py-2 rounded-lg font-bold hover:opacity-90 transition shadow-lg shadow-brand-accent/20"
            >Add Device</a
        >
    </div>

    <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        <!-- Device Card -->
        <div
            class="bg-brand-surface backdrop-blur border-2 border-brand-text/10 rounded-xl p-6 hover:border-brand-accent hover:shadow-lg hover:shadow-brand-accent/20 transition-all duration-300 cursor-pointer group"
        >
            <div class="flex justify-between items-start mb-4">
                <div
                    class="bg-green-500/20 text-green-500 px-2 py-1 rounded text-xs font-bold"
                >
                    ONLINE
                </div>
                <div class="text-brand-text/50">Battery: 85%</div>
            </div>
            <h3
                class="text-xl font-bold text-brand-text group-hover:text-brand-accent transition-colors mb-2"
            >
                Lecture Buddy v1
            </h3>
            <p class="text-brand-text/70 text-sm mb-6">Last seen: 2 mins ago</p>
            <div class="flex gap-2">
                <a
                    href="/panel/devices/123"
                    class="flex-1 bg-brand-text/5 hover:bg-brand-text/10 text-brand-text text-center py-2 rounded font-medium transition"
                    >Manage</a
                >
            </div>
        </div>

        <!-- Device Card -->
        <div
            class="bg-brand-surface backdrop-blur border-2 border-brand-text/10 rounded-xl p-6 hover:border-brand-accent hover:shadow-lg hover:shadow-brand-accent/20 transition-all duration-300 cursor-pointer group"
        >
            <div class="flex justify-between items-start mb-4">
                <div
                    class="bg-brand-text/10 text-brand-text/50 px-2 py-1 rounded text-xs font-bold"
                >
                    OFFLINE
                </div>
                <div class="text-brand-text/50">Battery: --</div>
            </div>
            <h3
                class="text-xl font-bold text-brand-text group-hover:text-brand-accent transition-colors mb-2"
            >
                Portable Rec 2
            </h3>
            <p class="text-brand-text/70 text-sm mb-6">Last seen: 2 days ago</p>
            <div class="flex gap-2">
                <a
                    href="/panel/devices/124"
                    class="flex-1 bg-brand-text/5 hover:bg-brand-text/10 text-brand-text text-center py-2 rounded font-medium transition"
                    >Manage</a
                >
            </div>
        </div>
    </div>
</div>
