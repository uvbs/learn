/*************************************************************************
    > File Name: stringi.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Sat 17 Jan 2015 12:01:43 PM CST
 ************************************************************************/
#include <stdio.h>

#include <string.h>
#include <assert.h>

namespace sudoku
{
    namespace character
    {
        typedef struct token_s {
            char * value;
            size_t length;
        } token_t;

        size_t tokenized_command(char * command, token_t * tokens, const size_t max_tokens)
        {
            char *s, *e;
            size_t ntokens = 0;
            size_t len = strlen(command);
            unsigned int i = 0;

            assert(command != NULL && tokens != NULL && max_tokens > 1);

            s = e = command;
            for (i = 0; i < len; i++) {
                if (*e == ' ') {
                    if (s != e) {
                        tokens[ntokens].value = s;
                        tokens[ntokens].length = e - s;
                        ntokens ++;
                        * e = '\0';

                        if (ntokens == max_tokens - 1) {
                            e ++;
                            s = e;
                            break;
                        }
                    }
                    s = e + 1;
                }
                e ++;
            }

            if (s != e) {
                tokens[ntokens].value = s;
                tokens[ntokens].length = e - s;
                ntokens ++;
            }

            tokens[ntokens].value = (*e == '\0' ? NULL : e);
            tokens[ntokens].length = 0;
            ntokens ++;

            return ntokens;
        }
    }
}

int main(int argc, char * argv[]) 
{
    char buf[100] = {0};
    gets(buf);
    printf("buf:%s\n", buf);

    sudoku::character::token_t tokens[10];
    size_t n = sudoku::character::tokenized_command(buf, tokens, 10);

    for (size_t i = 0; i < n; i++) {
        printf("token[%d] = %s\n", i, tokens[i]);
    }

    printf("end\n");

    return 0;
}
