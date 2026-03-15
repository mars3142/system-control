<script>
	function toRoman(num) {
		const lookup = {
			M: 1000,
			CM: 900,
			D: 500,
			CD: 400,
			C: 100,
			XC: 90,
			L: 50,
			XL: 40,
			X: 10,
			IX: 9,
			V: 5,
			IV: 4,
			I: 1,
		};
		let roman = "";
		for (let i in lookup) {
			while (num >= lookup[i]) {
				roman += i;
				num -= lookup[i];
			}
		}
		return roman;
	}

	const year = new Date().getFullYear();
	const romanYear = toRoman(year);
</script>

<footer class="px-0 py-4">
	<hr class="border-0 mb-4 border-t-2 border-solid" />
	<div class="flex justify-center items-center relative">
		<p class="m-0">© {romanYear} by mars3142</p>
		<p class="absolute right-0 m-0 text-sm text-gray-500">
			v{__APP_VERSION__} ({__COMMIT_HASH__})
		</p>
	</div>
</footer>
