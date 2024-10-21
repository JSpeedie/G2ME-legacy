passes=0
fails=0

PRODUCED_OUTPUT="produced_output"
EXPECTED_OUTPUT="correct_output"


testb1vo() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -b input/TEST1 > /dev/null

	TEST_NAME_LIST=(
		"testb1o"
		"testb1vo"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -o"
		"../G2ME -vo"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, appending the filepath for the output file in
		# the produced output directory
		$(${testcommand} ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


testb2vo() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -b input/TEST2 > /dev/null

	TEST_NAME_LIST=(
		"testb2o"
		"testb2vo"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -o"
		"../G2ME -vo"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, appending the filepath for the output file in
		# the produced output directory
		$(${testcommand} ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


testBvo() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testBo"
		"testBvo"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -o"
		"../G2ME -vo"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, appending the filepath for the output file in
		# the produced output directory
		$(${testcommand} ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


testb1vO() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -b input/TEST1 > /dev/null

	TEST_NAME_LIST=(
		"testb1O"
		"testb1vO"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -O"
		"../G2ME -vO"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


testb2vO() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -b input/TEST2 > /dev/null

	TEST_NAME_LIST=(
		"testb2O"
		"testb2vO"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -O"
		"../G2ME -vO"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


testBvO() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testBO"
		"testBvO"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -O"
		"../G2ME -vO"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


testM() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testM1"
		"testM2"
		"testM3"
		"testM4"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -M"
		"../G2ME -m 2 -M"
		"../G2ME -f input/filter1 -M"
		"../G2ME -f input/filter1 -m 2 -M"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


testc() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testc1"
		"testc2"
		"testc3"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -c Dylan"
		"../G2ME -c Michael"
		"../G2ME -c Valerie"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


testC() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testC1"
		"testC2"
		"testC3"
		"testC4"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -C"
		"../G2ME -m 2 -C"
		"../G2ME -f input/filter1 -C"
		"../G2ME -f input/filter1 -m 2 -C"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


testh() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testh1"
		"testh2"
		"testh3"
		"testh4"
		"testfvh1"
		"testfvh2"
		"testmvh1"
		"testmvh2"
		"testmfvh"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -h Dylan"
		"../G2ME -h Michael"
		"../G2ME -vh Victoria"
		"../G2ME -vh Aiden"
		"../G2ME -f input/filter1 -vh Aiden"
		"../G2ME -f input/filter1 -vh Dylan"
		"../G2ME -m 2 -vh Valerie"
		"../G2ME -m 3 -vh Valerie"
		"../G2ME -m 2 -f input/filter1 -vh Michael"
	)

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
}


runalltests() {
	testb1vo
	testb2vo
	testBvo
	testb1vO
	testb2vO
	testBvO
	testM
	testc
	testC
	testh
}


runalltests
total=$((fails+passes))

if [ "$fails" -eq 0 ]; then
	echo "${passes}/${total} All tests passed!"
else
	echo "${passes}/${total} passed. $fails tests failed."
fi
