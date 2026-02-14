<script lang="ts">
    import { page } from "$app/stores";
</script>

<div class="mb-8">
    <a
        href="/panel/devices"
        class="text-gray-400 hover:text-white mb-4 inline-block"
        >&larr; Back to Devices</a
    >
    <h1 class="text-3xl font-bold text-white">Device Configuration</h1>
    <p class="text-gray-500">ID: {$page.params.id}</p>
</div>

<div class="grid grid-cols-1 lg:grid-cols-2 gap-8">
    <div class="bg-gray-900 border border-gray-800 p-6 rounded-xl">
        <h2 class="text-xl font-bold text-white mb-4">Status</h2>
        <div class="space-y-4">
            <div class="flex justify-between border-b border-gray-800 pb-2">
                <span class="text-gray-400">Connection</span>
                <span class="text-green-500 font-bold">Online</span>
            </div>
            <div class="flex justify-between border-b border-gray-800 pb-2">
                <span class="text-gray-400">Battery Level</span>
                <span class="text-white">85%</span>
            </div>
            <div class="flex justify-between border-b border-gray-800 pb-2">
                <span class="text-gray-400">Storage Used</span>
                <span class="text-white">12.4 GB / 32 GB</span>
            </div>
            <div class="flex justify-between border-b border-gray-800 pb-2">
                <span class="text-gray-400">Firmware Version</span>
                <span class="text-white">v2.1.0</span>
            </div>
        </div>
    </div>

    <div class="bg-gray-900 border border-gray-800 p-6 rounded-xl">
        <h2 class="text-xl font-bold text-white mb-4">Actions</h2>
        <div class="space-y-3">
            <button
                class="w-full bg-white/5 hover:bg-white/10 text-white font-medium py-3 rounded-lg border border-gray-700 transition"
                >Update Firmware</button
            >
            <button
                class="w-full bg-white/5 hover:bg-white/10 text-white font-medium py-3 rounded-lg border border-gray-700 transition"
                >Calibrate Audio</button
            >
            <button
                class="w-full bg-red-500/10 hover:bg-red-500/20 text-red-500 font-medium py-3 rounded-lg border border-red-500/30 transition"
                >Unlink Device</button
            >
        </div>
    </div>
</div>
