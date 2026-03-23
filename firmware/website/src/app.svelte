<script lang="ts">
	import Header from './components/header.svelte';
	import Footer from './components/footer.svelte';
	import Index from './routes/index.svelte';
	import Captive from './routes/captive.svelte';
	import Router, { location } from 'svelte-spa-router';
	import { lang } from './i18n/store';

	const routes = {
		'/': Index,
		'/control': Index,
		'/config': Index,
		// Fallback route
		'*': Index
	};

	$effect(() => {
		document.documentElement.lang = $lang;
	});
</script>

<div class="container mx-auto lg:max-w-2xl">
	<Header />

	<main class="py-8">
		<Router {routes} />
	</main>

	<Footer />
</div>
