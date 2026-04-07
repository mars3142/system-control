<script lang="ts">
	import Header from './components/header.svelte';
	import Footer from './components/footer.svelte';
	import Index from './routes/index.svelte';
	import Captive from './routes/captive.svelte';
	import Router from 'svelte-spa-router';
	import { lang } from './i18n/store';
	import { Toaster } from 'svelte-french-toast';

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

<Toaster/>
<div class="container mx-auto">
	<Header />

	<main class="py-8">
		<Router {routes} />
	</main>

	<Footer />
</div>
