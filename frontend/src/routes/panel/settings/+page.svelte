<script lang="ts">
    import { theme } from "$lib/stores/theme";
</script>

<h1 class="text-3xl font-bold text-brand-text mb-8">Settings</h1>
<div
    class="max-w-2xl bg-brand-bg border border-gray-800 p-8 rounded-xl space-y-8"
>
    <div
        class="flex items-center justify-between border-b border-gray-800 pb-6"
    >
        <div>
            <h3 class="font-bold text-brand-text text-lg">Appearance</h3>
            <p class="text-sm text-gray-500">
                Customize the look and feel of the application.
            </p>
        </div>
        <select
            bind:value={$theme}
            class="bg-black/50 border border-gray-700 text-white rounded p-2 text-sm"
        >
            <option value="dark">Dark Mode</option>
            <option value="light">Light Mode</option>
        </select>
    </div>

    <div
        class="flex items-center justify-between border-b border-gray-800 pb-6"
    >
        <div>
            <h3 class="font-bold text-brand-text text-lg">
                Email Notifications
            </h3>
            <p class="text-sm text-gray-500">
                Receive summaries via email when processing completes.
            </p>
        </div>
        <div
            class="relative inline-block w-12 h-6 align-middle select-none transition duration-200 ease-in"
        >
            <input
                type="checkbox"
                name="toggle"
                id="toggle"
                class="toggle-checkbox absolute block w-6 h-6 rounded-full bg-white border-4 appearance-none cursor-pointer left-0 top-0 transition-transform duration-200 checked:translate-x-full checked:border-brand-accent"
            />
            <label
                for="toggle"
                class="toggle-label block overflow-hidden h-6 rounded-full bg-gray-700 cursor-pointer"
            ></label>
        </div>
    </div>

    <div class="flex items-center justify-between pb-2">
        <div>
            <h3 class="font-bold text-lg text-red-500">Danger Zone</h3>
            <p class="text-sm text-gray-500">
                Permanently delete your account and all data.
            </p>
        </div>
        <button
            class="bg-red-500/10 hover:bg-red-500/20 text-red-500 border border-red-500/30 px-4 py-2 rounded font-medium transition"
            >Delete Account</button
        >
    </div>
</div>
