<script lang="ts">
	import { t } from '../../i18n/store';
	import Card from '../common/card.svelte';
	import Toggle from '../common/toggle.svelte';

	let {
		lightOn = $bindable(false),
		thunderOn = $bindable(false),
		onchange,
	}: {
		lightOn?: boolean;
		thunderOn?: boolean;
		onchange: (e: CustomEvent<boolean>) => void;
	} = $props();

	function toggleLight(checked: boolean) {
		onchange(new CustomEvent('changeLight', { detail: checked }));
	}

	function toggleThunder(checked: boolean) {
		thunderOn = checked;
		// TODO: Send command to backend
	}
</script>

<Card>
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
</Card>
