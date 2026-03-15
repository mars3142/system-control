<script lang="ts">
	import { onMount } from "svelte";
	import { t } from "./i18n/store";
	import ControlTab from "./components/control/controlTab.svelte";
	import ConfigTab from "./components/config/configTab.svelte";
	import TabButton from "./components/common/tabButton.svelte";
	import TabBar from "./components/common/tabBar.svelte";
	import { controlStore } from "./stores/controlStore";

	let activeTab = "control";

	onMount(() => {
		controlStore.fetchState();
		const handleHashChange = () => {
			const hash = window.location.hash.slice(1);
			if (hash === "config") {
				activeTab = "config";
			} else {
				activeTab = "control";
			}
		};

		handleHashChange();
		window.addEventListener("hashchange", handleHashChange);

		return () => {
			window.removeEventListener("hashchange", handleHashChange);
		};
	});

	function setTab(tab: string) {
		activeTab = tab;
		window.location.hash = tab;
	}
</script>

<TabBar>
	<TabButton
		active={activeTab === "control"}
		label={$t("tab.control.title")}
		onClick={() => setTab("control")}
	/>
	<TabButton
		active={activeTab === "config"}
		label={$t("tab.config.title")}
		onClick={() => setTab("config")}
	/>
</TabBar>

<div class="tab-content">
	{#if activeTab === "control"}
		<ControlTab />
	{:else if activeTab === "config"}
		<ConfigTab />
	{/if}
</div>
