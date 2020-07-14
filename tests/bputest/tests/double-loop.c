# define OutLoopNum 20
# define InnerLoopNum 20

int main () {
    int a=0,b=0;
    for (int i = 0; i < OutLoopNum; i++) {
        for (int j = 1; j < InnerLoopNum; j++) {
            b = a + 1;
            a = b + 1;
        }
    }
    return 0;
}