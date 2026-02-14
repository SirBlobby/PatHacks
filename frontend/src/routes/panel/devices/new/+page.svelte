<div class="max-w-2xl mx-auto w-full">
    <div class="flex items-center gap-4 mb-8">
        <a
            href="/panel/devices"
            class="text-brand-text/50 hover:text-brand-text transition flex items-center gap-1 group"
        >
            <span class="group-hover:-translate-x-1 transition-transform"
                >←</span
            > Back</a
        >
        <h1 class="text-3xl font-bold text-brand-text">Register New Device</h1>
    </div>

    <div
        class="bg-brand-surface backdrop-blur border-2 border-brand-text/10 p-8 rounded-xl shadow-lg"
    >
        <form class="space-y-6">
            <div>
                <label
                    for="serial-number"
                    class="block text-brand-text/70 text-sm font-bold mb-2 ml-1"
                    >Device Serial Number</label
                >
                <input
                    id="serial-number"
                    type="text"
                    class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner"
                    placeholder="Enter S/N found on the back of device"
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
                    class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner"
                    placeholder="e.g. My Lecture Buddy"
                />
            </div>

            <div>
                <label
                    for="device-type"
                    class="block text-brand-text/70 text-sm font-bold mb-2 ml-1"
                    >Device Type</label
                >
                <select
                    id="device-type"
                    class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner"
                >
                    <option>Standard Recorder</option>
                    <option>Pro Model</option>
                </select>
            </div>

            <div class="pt-4">
                <button
                    type="submit"
                    class="w-full bg-brand-accent text-black font-bold py-3 rounded-lg hover:scale-[1.02] active:scale-[0.98] transition-transform shadow-lg shadow-brand-accent/20 cursor-pointer"
                >
                    Pair Device
                </button>
            </div>
        </form>
    </div>
</div>
