<script lang="ts">
	import { slide } from "svelte/transition";

	let { options = [], value = $bindable(), id = "", onchange }: { options?: { value: string; label: string }[], value: string, id?: string, onchange?: (e: CustomEvent<string>) => void } = $props();

	let isDropdownOpen = $state(false);
	let dropdownRef: HTMLDivElement;

	function toggleDropdown() {
		isDropdownOpen = !isDropdownOpen;
	}

	function selectOption(selectedValue: string) {
		value = selectedValue;
		isDropdownOpen = false;
		if (onchange) {
			onchange(new CustomEvent("change", { detail: selectedValue }));
		}
	}

	function handleClickOutside(event: MouseEvent) {
		if (dropdownRef && !dropdownRef.contains(event.target as Node)) {
			isDropdownOpen = false;
		}
	}

	// Re-evaluate label if value changes from outside
	let selectedLabel = $derived(options.find((o) => o.value === value)?.label || value);
</script>

<svelte:window onclick={handleClickOutside} />

<div class="relative w-full" bind:this={dropdownRef}>
	<button
		type="button"
		{id}
		class="w-full flex items-center justify-between p-4 rounded-md border bg-background text-foreground transition-all {isDropdownOpen
			? 'bg-primary/10 border-primary ring-destructive/20'
			: 'border-border hover:bg-accent'}"
		onclick={toggleDropdown}
		aria-haspopup="listbox"
		aria-expanded={isDropdownOpen}
	>
		<span class="font-medium">{selectedLabel}</span>
		<svg
			class="w-5 h-5 opacity-70 transition-transform duration-200"
			class:rotate-180={isDropdownOpen}
			fill="none"
			stroke="currentColor"
			viewBox="0 0 24 24"
			xmlns="http://www.w3.org/2000/svg"
		>
			<path
				stroke-linecap="round"
				stroke-linejoin="round"
				stroke-width="2"
				d="M19 9l-7 7-7-7"
			></path>
		</svg>
	</button>

	{#if isDropdownOpen}
		<ul
			transition:slide={{ duration: 150 }}
			class="absolute z-20 w-full mt-2 bg-popover text-popover-foreground bg-card border border-border rounded-md shadow-md overflow-hidden"
			role="listbox"
		>
			{#each options as option}
				<li>
					<button
						type="button"
						class="w-full text-left px-4 py-3 text-sm transition-colors hover:bg-accent hover:text-accent-foreground {option.value ===
						value
							? 'bg-primary text-primary-foreground font-medium'
							: ''}"
						role="option"
						aria-selected={option.value === value}
						onclick={() => selectOption(option.value)}
					>
						{option.label}
					</button>
				</li>
			{/each}
		</ul>
	{/if}
</div>
