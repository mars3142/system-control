<script lang="ts">
	import Card from '../common/card.svelte';
	import { t } from '../../i18n/store';
	import DropDown from '../common/dropDown.svelte';
	import Button from '../common/button.svelte';
	import toast from 'svelte-french-toast';
	import SchemaRow from './schemaRow.svelte';
	import { schemaStore, type SchemaRow as SchemaRowData } from '../../stores/configSchemaStore';
	import { controlStore } from '../../stores/controlStore';
	import { onMount } from 'svelte';

	const schemas = [
		{ value: 'schema_01.csv', label: $t('schema.name.1') },
		{ value: 'schema_02.csv', label: $t('schema.name.2') },
		{ value: 'schema_03.csv', label: $t('schema.name.3') }
	];

	let activeSchema = $state($controlStore.schema ?? 'schema_01.csv');
	let activeLabel = $derived(schemas.find((s) => s.value === activeSchema)?.label ?? activeSchema);
	let userSelected = $state(false);
	let currentSchema = $derived($controlStore.schema);

	$effect(() => {
		if (currentSchema && !userSelected) {
			activeSchema = currentSchema;
			schemaStore.fetchSchema(currentSchema).catch(() => {});
		}
	});

	let rows = $state<SchemaRowData[]>([]);

	onMount(() => {
		return schemaStore.subscribe((data) => {
			rows = data.map((r) => ({ ...r }));
		});
	});

	function rowToTime(index: number): string {
		const total = index * 30;
		const h = Math.floor(total / 60);
		const m = total % 60;
		return `${h < 10 ? '0' : ''}${h}:${m < 10 ? '0' : ''}${m}`;
	}

	async function loadClick() {
		try {
			await schemaStore.fetchSchema(activeSchema);
			toast.success($t('schema.loaded').replace('{file}', activeLabel));
		} catch {
			toast.error($t('schema.demo'));
		}
	}

	async function saveClick() {
		try {
			await schemaStore.saveSchema(activeSchema, rows);
			toast.success($t('schema.saved').replace('{file}', activeLabel));
		} catch {
			toast.error($t('error'));
		}
	}
</script>

<Card title="schema.editor.title">
	<div class="space-y-2">
		<div class="flex items-center justify-center cursor-pointer space-x-2">
			<DropDown
				id="active-schema"
				options={schemas}
				bind:value={activeSchema}
				onchange={() => (userSelected = true)}
			/>
			<Button
				label={$t('btn.load')}
				ariaLabel={$t('btn.load')}
				onClick={loadClick}
				icon="🔄" />
			<Button
				label={$t('btn.save')}
				ariaLabel={$t('btn.save')}
				onClick={saveClick}
				icon="💾" />
		</div>
		<div class="overflow-auto max-h-80 scrollbar">
			<table class="w-full table-fixed border-collapse text-left">
				<colgroup>
					<col class="w-14" />
					<col class="w-14" />
					<col />
					<col />
					<col />
					<col />
				</colgroup>
				<thead class="sticky top-0 z-10 bg-card">
					<tr class="border-b-2 border-border">
						<th class="px-3 py-2 text-sm">{$t('schema.header.time')}</th>
						<th class="px-3 py-2 text-sm text-center">{$t('schema.header.color')}</th>
						<th class="px-3 py-2 text-sm text-center">{$t('schema.header.red')}</th>
						<th class="px-3 py-2 text-sm text-center">{$t('schema.header.green')}</th>
						<th class="px-3 py-2 text-sm text-center">{$t('schema.header.blue')}</th>
						<th class="px-3 py-2 text-sm text-center">{$t('schema.header.brightness')}</th>
					</tr>
				</thead>
				<tbody>
					{#each rows as row, i}
						<SchemaRow
							time={rowToTime(i)}
							bind:r={row.r}
							bind:g={row.g}
							bind:b={row.b}
							bind:brightness={row.brightness}
						/>
					{/each}
					{#if rows.length === 0}
						<tr>
							<td colspan="6" class="px-3 py-4 text-sm text-center text-text-muted">
								{$t('schema.loading')}
							</td>
						</tr>
					{/if}
				</tbody>
			</table>
		</div>
	</div>
</Card>

<style>
    .scrollbar {
        scrollbar-color: var(--primary) var(--border);
        scrollbar-width: thin;
    }
    .scrollbar::-webkit-scrollbar {
        width: 6px;
    }
    .scrollbar::-webkit-scrollbar-track {
        background: var(--border);
        border-radius: 3px;
    }
    .scrollbar::-webkit-scrollbar-thumb {
        background: var(--primary);
        border-radius: 3px;
    }
</style>
