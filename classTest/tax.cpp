/*************************************************************************
    > File Name: tax.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Tue 10 Feb 2015 04:05:41 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

const int fast[] = {
   0,
   105,
   555,
   1005,
   2755,
   5505,
   13505,
   -1,
};

const double tax_percent[] = {
    0.03f,
    0.10f,
    0.20f,
    0.25f,
    0.30f,
    0.35f,
    0.45f,
    -1,
};

const double salary_stage[] = {
    1500,
    4500,
    9000,
    35000,
    55000,
    80000,
    -1,
};

int get_stage(double salary)
{
    int stage = 0;

    if (salary < 0) {
        stage = -1;
    } else if (salary <= salary_stage[0]) {
        stage = 0;
    } else if (salary <= salary_stage[1]) {
        stage = 1;
    } else if (salary <= salary_stage[2]) {
        stage = 2;
    } else if (salary <= salary_stage[3]) {
        stage = 3;
    } else if (salary <= salary_stage[4]) {
        stage = 4;
    } else if (salary <= salary_stage[5]) {
        stage = 5;
    } else {
        stage = 6;
    }

#if 0
    printf("get_stage, salary = %.5f, fast = %d, percent = %.3f\n", 
        salary, fast[stage], tax_percent[stage]);
#endif

    return stage;
}

int main(int argc, char * argv[]) 
{
    if (argc < 2) {
        printf("enter salary..\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        //double salary = atof(argv[i]) + 3500;
#if 0
        double salary = atof(argv[i]);// + 3500;
        if (salary < 0) {
            printf("invalid salary:%f\n", salary);
            continue;
        }
        int stage = get_stage(salary - 3500);
        double tax = 0;
        if (salary > 3500) {
            tax = (salary - 3500) * tax_percent[stage] - fast[stage];
        }

        printf("salary = %.5f, stage = %d, tax = %.5f, after = %.5f\n",
            salary, 
            stage,
            tax,
            salary - tax);
#else
        //calculate Annual bonus
        double money = atof(argv[i]);
        if (money < 0) {
            printf("invalid annual bonus:%f\n", money);
            continue;
        }
        double money_per_month = money / 12.0;
        int stage = get_stage(money_per_month);
        double tax = money * tax_percent[stage] - fast[stage];
        
        printf("money = %.2f, stage = %d, tax = %.2f, after = %.2f\n",
            money, 
            stage,
            tax,
            money - tax);
#endif
    }

    return 0;
}
