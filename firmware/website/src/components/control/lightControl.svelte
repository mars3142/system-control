<script lang="ts">
	import { t } from "../../i18n/store";
	import Toggle from "../common/toggle.svelte";

	export let lightOn = false;
	export let thunderOn = false;

	function toggleLight(checked: boolean) {
		lightOn = checked;
		// TODO: Send command to backend
	}

	function toggleThunder(checked: boolean) {
		thunderOn = checked;
		// TODO: Send command to backend
	}
</script>

<div class="bg-card p-6 rounded-lg border border-border shadow-sm">
	<h2 class="text-lg font-semibold mb-4 flex items-center gap-2">
		💡 {$t("control.light.title")}
	</h2>

	<div class="flex flex-col gap-4">
		<Toggle
			bind:checked={lightOn}
			label={$t("control.light.light")}
			onchange={toggleLight}
		/>
		<div class="hidden">
			<Toggle
				bind:checked={thunderOn}
				label={$t("control.light.thunder")}
				onchange={toggleThunder}
			/>
		</div>
	</div>
</div>
