CC=g++
CFLAGS=-Wall -Wextra -g
CPPFLAGS=-Wall -Wextra -g

all: typescript2txt

typescript2txt: typescript2txt.o

tests/01_passed: ./typescript2txt tests/01_input.txt tests/01_expected_output.txt
	@./typescript2txt < tests/01_input.txt > tests/01_actual_output.txt
	@diff -q tests/01_expected_output.txt tests/01_actual_output.txt
	touch tests/01_passed

tests/02_passed: ./typescript2txt tests/02_tabs_input.txt tests/02_tabs_expected_output.txt
	@./typescript2txt < tests/02_tabs_input.txt > tests/02_tabs_actual_output.txt
	@diff -q tests/02_tabs_expected_output.txt tests/02_tabs_actual_output.txt
	touch tests/02_passed

tests/03_passed: ./typescript2txt tests/03_tabs_input.txt tests/03_tabs_expected_output.txt
	@./typescript2txt < tests/03_tabs_input.txt > tests/03_tabs_actual_output.txt
	@diff -q tests/03_tabs_expected_output.txt tests/03_tabs_actual_output.txt
	touch tests/03_passed

tests/04_passed: ./typescript2txt tests/04_esc_D_input.txt tests/04_esc_D_expected_output.txt
	@./typescript2txt < tests/04_esc_D_input.txt > tests/04_esc_D_actual_output.txt
	@diff -q tests/04_esc_D_expected_output.txt tests/04_esc_D_actual_output.txt
	touch tests/04_passed

tests/05_passed: ./typescript2txt tests/05_esc_E_input.txt tests/05_esc_E_expected_output.txt
	@./typescript2txt < tests/05_esc_E_input.txt > tests/05_esc_E_actual_output.txt
	@diff -q tests/05_esc_E_expected_output.txt tests/05_esc_E_actual_output.txt
	touch tests/05_passed

tests/06_passed: ./typescript2txt tests/06_esc_M_input.txt tests/06_esc_M_expected_output.txt
	@./typescript2txt < tests/06_esc_M_input.txt > tests/06_esc_M_actual_output.txt
	@diff -q tests/06_esc_M_expected_output.txt tests/06_esc_M_actual_output.txt
	touch tests/06_passed

tests/07_passed: ./typescript2txt tests/07_OSC_0_setwi_input.txt tests/07_OSC_0_setwi_expected_output.txt
	@./typescript2txt < tests/07_OSC_0_setwi_input.txt > tests/07_OSC_0_setwi_actual_output.txt
	@diff -q tests/07_OSC_0_setwi_expected_output.txt tests/07_OSC_0_setwi_actual_output.txt
	touch tests/07_passed

tests/08_passed: ./typescript2txt tests/08_insert_blank_input.txt tests/08_insert_blank_expected_output.txt
	@./typescript2txt < tests/08_insert_blank_input.txt > tests/08_insert_blank_actual_output.txt
	@diff -q tests/08_insert_blank_expected_output.txt tests/08_insert_blank_actual_output.txt
	touch tests/08_passed

tests/09_passed: ./typescript2txt tests/09_insert_blank_input.txt tests/09_insert_blank_expected_output.txt
	@./typescript2txt < tests/09_insert_blank_input.txt > tests/09_insert_blank_actual_output.txt
	@diff -q tests/09_insert_blank_expected_output.txt tests/09_insert_blank_actual_output.txt
	touch tests/09_passed

tests/10_passed: ./typescript2txt tests/10_insert_blank_input.txt tests/10_insert_blank_expected_output.txt
	@./typescript2txt < tests/10_insert_blank_input.txt > tests/10_insert_blank_actual_output.txt
	@diff -q tests/10_insert_blank_expected_output.txt tests/10_insert_blank_actual_output.txt
	touch tests/10_passed

tests/11_passed: ./typescript2txt tests/11_insert_blank_input.txt tests/11_insert_blank_expected_output.txt
	@./typescript2txt < tests/11_insert_blank_input.txt > tests/11_insert_blank_actual_output.txt
	@diff -q tests/11_insert_blank_expected_output.txt tests/11_insert_blank_actual_output.txt
	touch tests/11_passed

tests/12_passed: ./typescript2txt tests/12_CSI_A_input.txt tests/12_CSI_A_expected_output.txt
	@./typescript2txt < tests/12_CSI_A_input.txt > tests/12_CSI_A_actual_output.txt
	@diff -q tests/12_CSI_A_expected_output.txt tests/12_CSI_A_actual_output.txt
	touch tests/12_passed

tests/13_passed: ./typescript2txt tests/13_CSI_A_input.txt tests/13_CSI_A_expected_output.txt
	@./typescript2txt < tests/13_CSI_A_input.txt > tests/13_CSI_A_actual_output.txt
	@diff -q tests/13_CSI_A_expected_output.txt tests/13_CSI_A_actual_output.txt
	touch tests/13_passed

tests/14_passed: ./typescript2txt tests/14_CSI_A_input.txt tests/14_CSI_A_expected_output.txt
	@./typescript2txt < tests/14_CSI_A_input.txt > tests/14_CSI_A_actual_output.txt
	@diff -q tests/14_CSI_A_expected_output.txt tests/14_CSI_A_actual_output.txt
	touch tests/14_passed

tests/15_passed: ./typescript2txt tests/15_CSI_A_input.txt tests/15_CSI_A_expected_output.txt
	@./typescript2txt < tests/15_CSI_A_input.txt > tests/15_CSI_A_actual_output.txt
	@diff -q tests/15_CSI_A_expected_output.txt tests/15_CSI_A_actual_output.txt
	touch tests/15_passed

tests/16_passed: ./typescript2txt tests/16_CSI_AB_input.txt tests/16_CSI_AB_expected_output.txt
	@./typescript2txt < tests/16_CSI_AB_input.txt > tests/16_CSI_AB_actual_output.txt
	@diff -q tests/16_CSI_AB_expected_output.txt tests/16_CSI_AB_actual_output.txt
	touch tests/16_passed

tests/17_passed: ./typescript2txt tests/17_CSI_B_input.txt tests/17_CSI_B_expected_output.txt
	@./typescript2txt < tests/17_CSI_B_input.txt > tests/17_CSI_B_actual_output.txt
	@diff -q tests/17_CSI_B_expected_output.txt tests/17_CSI_B_actual_output.txt
	touch tests/17_passed

tests/18_passed: ./typescript2txt tests/18_CSI_B_input.txt tests/18_CSI_B_expected_output.txt
	@./typescript2txt < tests/18_CSI_B_input.txt > tests/18_CSI_B_actual_output.txt
	@diff -q tests/18_CSI_B_expected_output.txt tests/18_CSI_B_actual_output.txt
	touch tests/18_passed

tests/19_passed: ./typescript2txt tests/19_CSI_C_input.txt tests/19_CSI_C_expected_output.txt
	@./typescript2txt < tests/19_CSI_C_input.txt > tests/19_CSI_C_actual_output.txt
	@diff -q tests/19_CSI_C_expected_output.txt tests/19_CSI_C_actual_output.txt
	touch tests/19_passed

tests/20_passed: ./typescript2txt tests/20_CSI_C_input.txt tests/20_CSI_C_expected_output.txt
	@./typescript2txt < tests/20_CSI_C_input.txt > tests/20_CSI_C_actual_output.txt
	@diff -q tests/20_CSI_C_expected_output.txt tests/20_CSI_C_actual_output.txt
	touch tests/20_passed

test: tests/02_passed tests/03_passed
test: tests/04_passed tests/05_passed tests/06_passed 
test: tests/07_passed tests/08_passed tests/09_passed
test: tests/10_passed tests/11_passed
test: tests/12_passed tests/13_passed tests/14_passed
test: tests/15_passed tests/16_passed tests/17_passed
test: tests/18_passed tests/19_passed tests/20_passed
test: #Tests after here are not expected to pass yet
test: tests/01_passed 

clean:
	-rm -f *.o typescript2txt 
	-rm -f tests/??_passed tests/??_*actual_output.txt

.PHONY: all clean test