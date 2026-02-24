.data
    buf:        .space 100
    fmt_str:    .asciz "|%s|\n"
    fmt_num:    .asciz "%d\n"
    msg_error:  .asciz "Error occured\n"

.text
.global main

# почти полностью повторяем add.c

main:
    # читаем ввод с помощью gets
    la a0, buf
    li a1, 100
    call gets

    # убираем '\n' в конце
    la t0, buf

remove_newline:
    lb t1, 0(t0)
    beqz t1, after_removing_newline
    li t2, '\n'
    beq t1, t2, set_zero
    addi t0, t0, 1
    j remove_newline

set_zero:
    sb x0, 0(t0)

after_removing_newline:
    # ниже полная проверка формата "%d %d"
    la s0, buf
    li s5, 0    # флаг найден ли первый набор цифр

check_first_digits:
    lb t0, 0(s0)
    beqz t0, error_exit
    li t1, '0'
    li t2, '9'
    blt t0, t1, error_exit
    bgt t0, t2, error_exit
    addi s0, s0, 1
    li s5, 1
    j check_first_digits_loop

check_first_digits_loop:
    lb t0, 0(s0)
    li t1, '0'
    li t2, '9'
    blt t0, t1, check_space
    bgt t0, t2, check_space
    addi s0, s0, 1
    j check_first_digits_loop

check_space:
    li t3, ' '
    bne t0, t3, error_exit
    addi s0, s0, 1
    beqz s5, error_exit

check_second_digits:
    lb t0, 0(s0)
    beqz t0, error_exit
    li t1, '0'
    li t2, '9'
    blt t0, t1, error_exit
    bgt t0, t2, error_exit
    addi s0, s0, 1
    j check_second_digits_loop

check_second_digits_loop:
    lb t0, 0(s0)
    beqz t0, format_ok
    li t1, '0'
    li t2, '9'
    blt t0, t1, error_exit
    bgt t0, t2, error_exit
    addi s0, s0, 1
    j check_second_digits_loop

format_ok:
    # логируем ввод: |buf| без \n
    la a0, fmt_str
    la a1, buf
    call printf

    # разделяем числа
    la s0, buf
    la s1, buf          # fst_p

find_space_split:
    lb t0, 0(s0)
    beq t0, x0, after_split   # вдруг нет пробела (не должно быть)
    li t1, ' '
    bne t0, t1, continue_split
    sb x0, 0(s0)        # заменяем пробел на '\0'
    addi s0, s0, 1
    j after_split

continue_split:
    addi s0, s0, 1
    j find_space_split

after_split:
    mv s2, s0           # snd_p

    la a0, buf
    call atoi
    mv s3, a0           # a = atoi(fst_p)

    mv a0, s2
    call atoi
    mv s4, a0           # b = atoi(snd_p)

    add s5, s3, s4      # sum = a + b

    la a0, fmt_num
    mv a1, s5
    call printf         # printf(fmt_num, sum)

    li a0, 0
    li a7, 2            # exit(0)
    ecall

error_exit:
    la a0, msg_error
    call printf
    li a0, 1
    li a7, 2            # exit(1)
    ecall
