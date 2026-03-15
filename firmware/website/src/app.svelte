<script lang="ts">
	import Header from "./components/header.svelte";
	import Footer from "./components/footer.svelte";
	import Index from "./index.svelte";
	import Captive from "./captive.svelte";
	import Router, { location } from "svelte-spa-router";
	import { lang } from "./i18n/store";

	const routes = {
		"/control": Index,
		"/captive": Captive,
		// Fallback route
		"*": Index,
	};

	$: document.documentElement.lang = $lang;
</script>

{#if $location !== "/captive"}
	<Header />
{/if}

<main class="container mx-auto px-4 py-8">
	<Router {routes} />
</main>

{#if $location !== "/captive"}
	<Footer />
{/if}
