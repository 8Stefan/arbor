for cpr in 100 150 225 338 422 527 659 824 1030 1287 1609 2012 2515 3772 5658 8487 12730
#for cpr in 1000
do
	for ns in 10 12 14 17 21 25 30 36 43 52 62 74 89 107 128 154 185 222 266 319 383 460 552 662 795 954 1145 1374 1648 1978 2374 2849 3418 4102 4922 5907 7088 8506 10207
	do
		echo ==== 1 rank with $cpr cells per rank, each with $ns synapses
		name=run_nr_1_cpr_${cpr}_ns_${ns}
		cp params_base.json params_$name.json   
		sed -i "s|cpr|${cpr}|g" params_$name.json
		sed -i "s|ns|${ns}|g" params_$name.json
		./bin/drybench params_$name.json > out_$name
	done
done
