//禁止規則４

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define CITY 197
#define REPETATION 10000
#define NEIGHBOR 50
#define TABU 4

#define EXECUTE 100

typedef struct{
    int keiro[CITY];
    float fitness;
}Individual;

typedef struct{
    int num;
    float x, y;
}Position;

typedef struct{
    int soeji1, soeji2;
    int yoso1, yoso2;
}TabuList;



//乱数生成
int getRandom(int min, int max){
    return min + (int)(rand()*(max-min+1.0)/(1.0+RAND_MAX));
}


//距離
float distance(Position* pos, Individual a){
    int i;
    double dis = 0.0;
    
    for(i=0;i<CITY-1;i++){
        dis += sqrt((double)((pos[a.keiro[i+1]-1].x-pos[a.keiro[i]-1].x)*(pos[a.keiro[i+1]-1].x-pos[a.keiro[i]-1].x)+(pos[a.keiro[i+1]-1].y-pos[a.keiro[i]-1].y)*(pos[a.keiro[i+1]-1].y-pos[a.keiro[i]-1].y)));
    }
    //distancce of [31]>>[0]
    dis += sqrt((double)((pos[a.keiro[CITY-1]-1].x-pos[a.keiro[0]-1].x)*(pos[a.keiro[CITY-1]-1].x-pos[a.keiro[0]-1].x)+(pos[a.keiro[CITY-1]-1].y-pos[a.keiro[0]-1].y)*(pos[a.keiro[CITY-1]-1].y-pos[a.keiro[0]-1].y)));
    
    
    return (float)dis;
}


//2都市交換
Individual swapTwo(Individual ind, int a, int b){
    int tmp;
    
    tmp = ind.keiro[a];
    ind.keiro[a] = ind.keiro[b];
    ind.keiro[b] = tmp;
    
    return ind;
}


//タブーリストに含まれているかチェック
int includeTabuList(TabuList* t, Individual a, int r1, int r2){
    int i;
    
    for(i=0;i<TABU;i++){
        if(a.keiro[r1]==t[i].yoso2 && r1 == t[i].soeji1 && a.keiro[r2]==t[i].yoso1 && r2 == t[i].soeji2){
            return 1;
        }
    }
    return 0;
}



//コストが良い上位TABU個を更新
void addTabuList(int rnd1, int rnd2, Individual bestAnswer, TabuList* tabu, int t){
    
    //tabu[10]まできたらtabu[0]に戻って保存
    
    tabu[t].soeji1 = rnd1;
    tabu[t].yoso1 = bestAnswer.keiro[rnd2];
    tabu[t].soeji2 = rnd2;
    tabu[t].yoso2 = bestAnswer.keiro[rnd1];
    
    /*printf("%d\n", t);          //確認用
    for(int i=0;i<TABU;i++){
        printf("TL: %d %d %d %d\n", tabu[i].soeji1, tabu[i].yoso1, tabu[i].soeji2, tabu[i].yoso2);
    }*/
}




int main(void) {
    FILE *fp, *fp1;
    
    Individual ind;
    Individual bestAnswer;
    Position pos[CITY];
    TabuList tabu[TABU] = {0};
    
    char file[] = "world_position.csv";
    char name1[100], name2[100], name3[100];
    int i, j;
    int ta = 0;
    int tmp;
    int rnd1;
    int rnd2;
    int a;
    
    //グラフ出力用
    float best = 0.0;
    float worst = 1.0;
    float average = 0.0;
    float cpu_ave = 0.0;
    
    clock_t start, end;
    double sec;
    
    //結果をファイルに出力
    char file1[] = "ts_data4.1.csv";
    fp1 = fopen(file1, "w");
    if(fp1 == NULL){
        printf("%s Open Error\n", file1);
        return -1;
    }
    
    fprintf(fp1, "No.,distance,CPUtime\n");
    

    //乱数の初期化
    srand((unsigned)time(NULL));
        
    fp = fopen(file, "r");
    if(fp==NULL){
        printf("%s Open Error\n", file);
        return -1;
    }
    
    fscanf(fp, "%[^,],%[^,],%s", name1, name2, name3);      //１行目
    for(int i=0;i<CITY;i++){
        fscanf(fp, "%d,%f,%f", &pos[i].num, &pos[i].x, &pos[i].y);        //２行目以降
    }
    fclose(fp);

    
    for(a=0;a<EXECUTE;a++){
        
        fprintf(fp1, "exe:%d\n", a+1);
        
        start = clock();

        
        //初期解
        for(i=0;i<CITY;i++){
            bestAnswer.keiro[i] = i+1;
        }
        bestAnswer.fitness = 1/distance(pos, bestAnswer);
        
        
        //タブーサーチ　禁止規則４
        for(i=0;i<REPETATION;i++){
        
            ind = bestAnswer;
                        
            //近傍解生成
            for(j=0;j<NEIGHBOR;j++){
                
                //2都市入れ替え
                rnd1 = 0;
                rnd2 = 0;
                 
                while(rnd1==rnd2){
                    rnd1 = getRandom(1,CITY-1);
                    rnd2 = getRandom(1,CITY-1);
                }
                
                
                if(rnd1>rnd2){
                    tmp = rnd1;
                    rnd1 = rnd2;
                    rnd2 = tmp;
                }
                

                
                ind = swapTwo(ind, rnd1, rnd2);
                ind.fitness = 1/distance(pos, ind);
                
                
                //評価
                if(ind.fitness>bestAnswer.fitness){
                    
                    //タブーリストにないとき
                    if(!includeTabuList(tabu, ind, rnd1, rnd2)){
                        
                        //最良解更新
                        bestAnswer = ind;
                        
                        //タブーリストに追加
                        if(ta==TABU){
                            ta=0;
                        }
                        addTabuList(rnd1, rnd2, bestAnswer, tabu, ta);
                        ta++;
                        
                        
                    }/*else{
                        printf("included TabuList\n");      //確認用
                    }
                    printf("--------------------\n");*/
                    
                }
                
                //元に戻す
                ind = swapTwo(ind, rnd1, rnd2);
            }
            
            fprintf(fp1, "%d,%f\n", i, 1/bestAnswer.fitness);

            
        }
        
        
        end = clock();
        sec = (double)(end-start)/CLOCKS_PER_SEC;
        
        //出力
        fprintf(fp1, "%d,%lf,%lf\n\n", a+1, 1/bestAnswer.fitness, sec);
        
        cpu_ave += sec;
        
        average += 1/bestAnswer.fitness;
        
        if(best<bestAnswer.fitness){
            best = bestAnswer.fitness;
        }
        if(worst>bestAnswer.fitness){
            worst = bestAnswer.fitness;
        }
        
    }
    cpu_ave = cpu_ave/EXECUTE;
    average = average/EXECUTE;
    best = 1/best;
    worst = 1/worst;
    
    //出力
    fprintf(fp1, "\nbest,worst,average,CPUave,Tabu\n");
    fprintf(fp1, "%f,%f,%f,%f,%d\n", best, worst, average, cpu_ave, TABU);
    
    fclose(fp1);
    
    
    //表示
    printf("\n反復回数：%d回\n", REPETATION);
    printf("最短距離：%lf\n", 1/bestAnswer.fitness);
    printf("最短経路：");
    for(int i=0;i<CITY;i++){
        printf("%d ", bestAnswer.keiro[i]);
    }
    printf("\n");
    
    return 0;
}


