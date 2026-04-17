<script lang="ts">
	import { onMount } from "svelte";
	import { location, replace } from 'svelte-spa-router';
	import { t } from "../i18n/store";
	import { controlStore } from "../stores/controlStore";
	import ControlTab from "../components/controlTab/controlTab.svelte";
	import ConfigTab from "../components/configTab/configTab.svelte";
	import SystemTab from "../components/systemTab/systemTab.svelte";
	import TabButton from "../components/common/tabButton.svelte";
	import TabBar from "../components/common/tabBar.svelte";

	type Tab = "control" | "config" | "system";

	const tabToPath: Record<Tab, string> = {
		control: "/control",
		config: "/config",
		system: "/system"
	};

	function pathToTab(path: string): Tab {
		if (path === "/config") return "config";
		if (path === "/system") return "system";
		return "control";
	}

	let activeTab = $derived(pathToTab($location));

	onMount(() => {
		controlStore.fetchState();

		// Optional: Default-Route
		if ($location === "/") {
			replace("/control");
		}
	});

	function setTab(tab: Tab) {
		replace(tabToPath[tab]);
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
	<TabButton
		active={activeTab === "system"}
		label={$t("tab.system.title")}
		onClick={() => setTab("system")}
	/>
</TabBar>

<div class="tab-content">
	{#if activeTab === "control"}
		<ControlTab />
	{:else if activeTab === "config"}
		<ConfigTab />
	{:else}
		<SystemTab />
	{/if}
</div>
