<script lang="ts">
	import "./layout.css";
	import favicon from "$lib/assets/favicon.svg";
	import DeskPet from "$lib/components/DeskPet.svelte";
	import CircuitBackground from "$lib/components/CircuitBackground.svelte";

	let { children } = $props();
</script>

<svelte:head>
	<link rel="icon" href={favicon} />
	<title>LearningBuddy</title>
</svelte:head>

<CircuitBackground />
<DeskPet />

{@render children()}
