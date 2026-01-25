<script lang="ts">
	import Header from "./compoents/Header.svelte";
	import Index from "./Index.svelte";
	import Captive from "./Captive.svelte";
	import { onMount } from "svelte";
	import { writable } from "svelte/store";

	const isCaptive = writable(false);

	function checkHash() {
		isCaptive.set(window.location.hash === "#/captive");
	}

	onMount(() => {
		checkHash();
		window.addEventListener("hashchange", checkHash);
		return () => window.removeEventListener("hashchange", checkHash);
	});
</script>

<Header />

{#if $isCaptive}
	<Captive />
{:else}
	<Index />
{/if}
