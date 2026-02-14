<div class="max-w-2xl mx-auto">
    <h1 class="text-3xl font-bold text-white mb-8">Register New Device</h1>

    <div class="bg-gray-900 border border-gray-800 p-8 rounded-xl shadow-2xl">
        <form class="space-y-6">
            <div>
                <label
                    for="serial-number"
                    class="block text-gray-400 text-sm font-medium mb-2"
                    >Device Serial Number</label
                >
                <input
                    id="serial-number"
                    type="text"
                    class="w-full bg-black/50 border border-gray-700 rounded-lg p-3 text-white focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none"
                    placeholder="Enter S/N found on the back of device"
                />
            </div>

            <div>
                <label
                    for="device-name"
                    class="block text-gray-400 text-sm font-medium mb-2"
                    >Device Name</label
                >
                <input
                    id="device-name"
                    type="text"
                    class="w-full bg-black/50 border border-gray-700 rounded-lg p-3 text-white focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none"
                    placeholder="e.g. My Lecture Buddy"
                />
            </div>

            <div>
                <label
                    for="device-type"
                    class="block text-gray-400 text-sm font-medium mb-2"
                    >Device Type</label
                >
                <select
                    id="device-type"
                    class="w-full bg-black/50 border border-gray-700 rounded-lg p-3 text-white focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none"
                >
                    <option>Standard Recorder</option>
                    <option>Pro Model</option>
                </select>
            </div>

            <div class="pt-4">
                <button
                    type="submit"
                    class="w-full bg-brand-accent text-black font-bold py-3 rounded-lg hover:opacity-90 transition"
                >
                    Pair Device
                </button>
            </div>
        </form>
    </div>
</div>
