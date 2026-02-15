<script lang="ts">
    import Sidebar from "$lib/components/Sidebar.svelte";
    import { page } from "$app/stores";
    import { goto } from "$app/navigation";
    import { onMount } from "svelte";
    import { isAuthenticated } from "$lib/api";
    import Icon from "@iconify/svelte";

    let { children } = $props();
    let isMobileMenuOpen = $state(false);
    let authChecked = $state(false);

    onMount(() => {
        if (!isAuthenticated()) {
            goto("/signin");
            return;
        }
        authChecked = true;
    });

    $effect(() => {
        // Close menu on navigation
        if ($page.url.pathname) {
            isMobileMenuOpen = false;
        }
    });
</script>

{#if authChecked}
    <div class="flex min-h-screen">
        <Sidebar isOpen={isMobileMenuOpen} />

        <!-- Mobile Header -->
        <div
            class="md:hidden fixed top-0 left-0 w-full z-40 bg-brand-surface/90 backdrop-blur border-b border-brand-text/10 p-4 flex items-center justify-between"
        >
            <span class="font-bold text-brand-accent">LearningBuddy</span>
            <button
                class="text-brand-text p-2"
                onclick={() => (isMobileMenuOpen = !isMobileMenuOpen)}
                aria-label="Toggle menu"
            >
                {#if isMobileMenuOpen}
                    <Icon icon="mdi:close" class="text-2xl" />
                {:else}
                    <Icon icon="mdi:menu" class="text-2xl" />
                {/if}
            </button>
        </div>

        <!-- Mobile Overlay -->
        {#if isMobileMenuOpen}
            <button
                class="fixed inset-0 bg-black/50 z-40 md:hidden backdrop-blur-sm"
                onclick={() => (isMobileMenuOpen = false)}
                aria-label="Close menu"
            ></button>
        {/if}

        <main
            class="flex-1 md:ml-64 p-8 relative z-10 w-full overflow-x-hidden mt-16 md:mt-0"
        >
            {@render children()}
        </main>
    </div>
{:else}
    <div class="flex items-center justify-center min-h-screen">
        <div
            class="w-10 h-10 border-4 border-brand-accent border-t-transparent rounded-full animate-spin"
        ></div>
    </div>
{/if}
