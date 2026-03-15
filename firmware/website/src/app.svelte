<script lang="ts">
	import Header from './components/header.svelte';
	import Footer from './components/footer.svelte';
	import Index from './routes/index.svelte';
	import Captive from './routes/captive.svelte';
	import Router, { location } from 'svelte-spa-router';
	import { lang } from './i18n/store';

	const routes = {
		'/': Index,
		'/captive': Captive,
		// Fallback route
		'*': Index
	};

	$effect(() => {
		document.documentElement.lang = $lang;
	});
</script>

<Header />

<main class="container mx-auto px-4 py-8">
	<Router {routes} />
</main>

{#if $location !== "/captive"}
	<Footer />
{/if}
