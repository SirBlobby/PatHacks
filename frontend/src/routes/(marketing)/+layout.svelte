<script lang="ts">
  import Navbar from '$lib/components/Navbar.svelte';
  let { children } = $props();
</script>

<Navbar />

<main class="container mx-auto pt-24 px-4 min-h-screen relative z-10 w-full">
  {@render children()}
</main>
