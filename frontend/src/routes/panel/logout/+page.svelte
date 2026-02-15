<script lang="ts">
    import { goto } from "$app/navigation";
    import { onMount } from "svelte";
    import { logout } from "$lib/api";

    onMount(() => {
        logout();
        setTimeout(() => {
            goto("/signin");
        }, 1500);
    });
</script>

<div class="flex flex-col items-center justify-center min-h-[50vh]">
    <div
        class="w-12 h-12 border-4 border-brand-accent border-t-transparent rounded-full animate-spin mb-4"
    ></div>
    <h2 class="text-2xl font-bold text-brand-text">Logging out...</h2>
    <p class="text-brand-text/50">See you next time!</p>
</div>
