<script lang="ts">
	import { t } from '../../i18n/store';
	import ModeButton from './modeButton.svelte';
	import DropDown from '../common/dropDown.svelte';
    import Card from '../common/card.svelte';

	let { mode = $bindable('simulation'), activeSchema = $bindable('schema_01.csv'), onchangeMode, onchangeSchema }: {
		mode?: string,
		activeSchema?: string,
		onchangeMode?: (e: CustomEvent<string>) => void,
		onchangeSchema?: (e: CustomEvent<string>) => void
	} = $props();

	let schemas = $derived([
		{ value: 'schema_01.csv', label: $t('schema.name.1') },
		{ value: 'schema_02.csv', label: $t('schema.name.2') },
		{ value: 'schema_03.csv', label: $t('schema.name.3') }
	]);

	function setMode(newMode: string) {
		mode = newMode;
		if (onchangeMode) {
			onchangeMode(new CustomEvent('changeMode', { detail: mode }));
		}
	}

	function handleSchemaChange(event: CustomEvent<string>) {
		// When the user selects a new schema from the DropDown, we update our local state.
		// If the update came from the WS, activeSchema would be updated directly by the parent.
		activeSchema = event.detail;
		if (onchangeSchema) {
			onchangeSchema(new CustomEvent('changeSchema', { detail: activeSchema }));
		}
	}
</script>

<Card>
<h2 class="text-lg font-semibold mb-4 flex items-center gap-2">
	🔄 {$t("control.mode.title")}
</h2>

<div class="flex gap-2 mb-6">
	<ModeButton active={mode === 'day'} icon="☀️" label={$t('mode.day')} onClick={() => setMode('day')} />
	<ModeButton active={mode === 'night'} icon="🌙" label={$t('mode.night')} onClick={() => setMode('night')} />
	<ModeButton active={mode === 'simulation'} icon="🔄" label={$t('mode.simulation')}
							onClick={() => setMode('simulation')} />
</div>

{#if mode === 'simulation'}
	<div class="p-4 bg-background rounded-md border border-border">
		<label for="active-schema" class="block text-sm font-medium mb-2">{$t("control.schema.active")}</label>

		<DropDown
			id="active-schema"
			options={schemas}
			bind:value={activeSchema}
			onchange={handleSchemaChange}
		/>
	</div>
{/if}
</Card>
