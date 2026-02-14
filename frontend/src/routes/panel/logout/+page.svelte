<script lang="ts">
    import { onMount } from "svelte";

    onMount(() => {
        // In a real app, clear tokens/cookies here
        setTimeout(() => {
            window.location.href = "/signin";
        }, 1500);
    });
</script>

<div class="flex flex-col items-center justify-center min-h-[50vh]">
    <div
        class="w-12 h-12 border-4 border-brand-accent border-t-transparent rounded-full animate-spin mb-4"
    ></div>
    <h2 class="text-2xl font-bold text-white">Logging out...</h2>
    <p class="text-gray-500">See you next time!</p>
</div>
