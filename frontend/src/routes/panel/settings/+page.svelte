<script lang="ts">
    import { onMount } from "svelte";
    import { goto } from "$app/navigation";
    import {
        getSettings,
        updateSettings,
        changePassword,
        deleteAccount,
    } from "$lib/api";
    import { theme } from "$lib/stores/theme";

    let loading = $state(true);
    let saving = $state(false);
    let error = $state("");
    let success = $state("");

    let selectedTheme = $state("dark");

    // Password change
    let currentPassword = $state("");
    let newPassword = $state("");
    let passwordError = $state("");
    let passwordSuccess = $state("");
    let changingPassword = $state(false);

    onMount(async () => {
        try {
            const settings = await getSettings();
            selectedTheme = settings.theme || "dark";
        } catch (err: any) {
            error = err.message;
        } finally {
            loading = false;
        }
    });

    async function handleSaveSettings() {
        saving = true;
        error = "";
        success = "";
        try {
            await updateSettings({
                theme: selectedTheme,
            });
            // Also update the local theme store
            theme.set(selectedTheme);
            success = "Settings saved!";
        } catch (err: any) {
            error = err.message;
        } finally {
            saving = false;
        }
    }

    async function handleChangePassword(e: Event) {
        e.preventDefault();
        changingPassword = true;
        passwordError = "";
        passwordSuccess = "";
        try {
            await changePassword(currentPassword, newPassword);
            passwordSuccess = "Password changed successfully!";
            currentPassword = "";
            newPassword = "";
        } catch (err: any) {
            passwordError = err.message;
        } finally {
            changingPassword = false;
        }
    }

    async function handleDeleteAccount() {
        if (
            !confirm(
                "Are you sure you want to delete your account? This action is permanent and cannot be undone.",
            )
        )
            return;
        if (
            !confirm(
                "This will delete ALL your data including sources, devices, and chat history. Continue?",
            )
        )
            return;

        try {
            await deleteAccount();
            localStorage.removeItem("auth_token");
            localStorage.removeItem("auth_user");
            goto("/");
        } catch (err: any) {
            alert(err.message);
        }
    }
</script>

<div class="max-w-2xl mx-auto w-full space-y-8">
    <h1 class="text-3xl font-bold text-brand-text">Settings</h1>

    {#if loading}
        <div class="flex justify-center py-20">
            <div
                class="w-10 h-10 border-4 border-brand-accent border-t-transparent rounded-full animate-spin"
            ></div>
        </div>
    {:else}
        <!-- Theme & Notifications -->
        <div
            class="bg-brand-surface backdrop-blur border-2 border-brand-text/10 p-8 rounded-xl shadow-lg"
        >
            <h2 class="text-xl font-bold text-brand-text mb-6">Preferences</h2>

            {#if success}
                <div
                    class="mb-4 p-3 bg-green-500/10 border border-green-500/30 rounded-lg text-green-500 text-sm text-center"
                >
                    {success}
                </div>
            {/if}
            {#if error}
                <div
                    class="mb-4 p-3 bg-red-500/10 border border-red-500/30 rounded-lg text-red-500 text-sm text-center"
                >
                    {error}
                </div>
            {/if}

            <div class="space-y-6">
                <div>
                    <label
                        for="theme"
                        class="block text-brand-text/70 text-sm font-bold mb-2"
                        >Theme</label
                    >
                    <select
                        id="theme"
                        bind:value={selectedTheme}
                        class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner"
                    >
                        <option value="dark">Dark</option>
                        <option value="light">Light</option>
                    </select>
                </div>

                <button
                    onclick={handleSaveSettings}
                    disabled={saving}
                    class="w-full bg-brand-accent text-black font-bold py-3 rounded-lg hover:scale-[1.02] active:scale-[0.98] transition-transform shadow-lg shadow-brand-accent/20 cursor-pointer disabled:opacity-50"
                >
                    {saving ? "Saving..." : "Save Preferences"}
                </button>
            </div>
        </div>

        <!-- Change Password -->
        <div
            class="bg-brand-surface backdrop-blur border-2 border-brand-text/10 p-8 rounded-xl shadow-lg"
        >
            <h2 class="text-xl font-bold text-brand-text mb-6">
                Change Password
            </h2>

            {#if passwordSuccess}
                <div
                    class="mb-4 p-3 bg-green-500/10 border border-green-500/30 rounded-lg text-green-500 text-sm text-center"
                >
                    {passwordSuccess}
                </div>
            {/if}
            {#if passwordError}
                <div
                    class="mb-4 p-3 bg-red-500/10 border border-red-500/30 rounded-lg text-red-500 text-sm text-center"
                >
                    {passwordError}
                </div>
            {/if}

            <form class="space-y-4" onsubmit={handleChangePassword}>
                <div>
                    <label
                        for="current-password"
                        class="block text-brand-text/70 text-sm font-bold mb-2"
                        >Current Password</label
                    >
                    <input
                        id="current-password"
                        type="password"
                        bind:value={currentPassword}
                        class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner"
                        placeholder="••••••••"
                        required
                    />
                </div>
                <div>
                    <label
                        for="new-password"
                        class="block text-brand-text/70 text-sm font-bold mb-2"
                        >New Password</label
                    >
                    <input
                        id="new-password"
                        type="password"
                        bind:value={newPassword}
                        class="w-full bg-brand-bg border border-brand-text/20 rounded-lg p-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all shadow-inner"
                        placeholder="••••••••"
                        required
                        minlength="6"
                    />
                </div>
                <button
                    type="submit"
                    disabled={changingPassword}
                    class="w-full py-3 rounded-lg border-2 border-brand-text/10 text-brand-text font-bold hover:bg-brand-text/5 transition-colors cursor-pointer disabled:opacity-50"
                >
                    {changingPassword ? "Changing..." : "Change Password"}
                </button>
            </form>
        </div>

        <!-- Danger Zone -->
        <div
            class="bg-brand-surface backdrop-blur border-2 border-red-500/20 p-8 rounded-xl shadow-lg"
        >
            <h2 class="text-xl font-bold text-red-500 mb-2">Danger Zone</h2>
            <p class="text-brand-text/50 text-sm mb-6">
                Permanently delete your account and all associated data. This
                cannot be undone.
            </p>
            <button
                onclick={handleDeleteAccount}
                class="px-6 py-3 rounded-lg bg-red-500/10 border border-red-500/30 text-red-500 font-bold hover:bg-red-500/20 transition-colors cursor-pointer"
            >
                Delete Account
            </button>
        </div>
    {/if}
</div>
