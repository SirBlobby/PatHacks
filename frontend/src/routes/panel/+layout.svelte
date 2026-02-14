<script lang="ts">
    import Sidebar from "$lib/components/Sidebar.svelte";
    import { page } from "$app/stores";

    let { children } = $props();
    let isMobileMenuOpen = $state(false);

    $effect(() => {
        // Close menu on navigation
        if ($page.url.pathname) {
            isMobileMenuOpen = false;
        }
    });
</script>

<div class="flex min-h-screen">
    <Sidebar isOpen={isMobileMenuOpen} />

    <!-- Mobile Header -->
    <div
        class="md:hidden fixed top-0 left-0 w-full z-40 bg-brand-bg/90 backdrop-blur border-b border-brand-text/10 p-4 flex items-center justify-between"
    >
        <span class="font-bold text-brand-accent">LearningBuddy</span>
        <button
            class="text-brand-text p-2"
            onclick={() => (isMobileMenuOpen = !isMobileMenuOpen)}
            aria-label="Toggle menu"
        >
            {#if isMobileMenuOpen}✕{:else}☰{/if}
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
