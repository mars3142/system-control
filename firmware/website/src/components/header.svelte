<script lang="ts">
	import Button from "./common/button.svelte";
	import { lang, t } from "../i18n/store";

	let currentLangCode = $state($lang);
	let currentLang = $state("English");
	let currentFlag = $state("🇬🇧");

	$effect(() => {
		lang.subscribe(($lang) => {
			currentLangCode = $lang;
			currentLang = $t("common.language");
			currentFlag = $t("common.flag");
		});
	});

	function handleLangChange(newLang: "de" | "en") {
		lang.set(newLang);

		localStorage.setItem("lang", newLang);
	}
</script>

<div class="flex flex-wrap justify-between items-center mb-5">
	<div>
		<Button
			ariaLabel="Sprache wechseln"
			icon={currentFlag}
			label={currentLang}
			onClick={() => {
				const newLang = currentLangCode === "de" ? "en" : "de";
				handleLangChange(newLang);
			}}
		/>
	</div>
	<h1 class="font-bold order-first text-2xl text-primary dark:text-text">
		🚂 System Control
	</h1>
</div>
