//2022.12.8 Ryusei Ishikawa Updated.
//2022.12.11 matubara Updated
//2022.12.11 Ryusei Ishikawa Updated.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "best_params.h"
#define ALPHA_BETA_FUNC_NAME Alpha_Beta

//READ HERE:
//下記のglobal変数群を使いながら、次の関数に渡して最適手の計算を行うことを考えています。
//引数sideは0の時先手(盤の手前側)の手番、0の時後手(盤の奥側)の手番
//返り値はユーザの入力と同様、"2A3B"などの文字列
//引数と返り値に関しては変更するにはコードの手直しが要るので容易ではないですが、関数名については4行目のALPHA_BETA_FUNC_NAMEのマクロの定義を変えてもらえるだけで違う関数名でも動きますので、変更したい場合はどうぞ。
char* Alpha_Beta(int side);
//global変数群
//0のときUser、1のときAIが先手となる。2のときいずれもUser、3のときいずれもAI。2及び3はdebug用。
int First_Player=3;
//0の時は通常、1の時は盤面を表示する。
int Display_Board_Enabled=0;
//盤及び持ち駒による表示。
//boardは盤、possessedは先手および後手の持ち駒を表す。
//'x'は該当するコマがないことを表す。boardにおいては一番内側の二要素の配列は{コマの種類, 向き}となっている。
//'F'は歩兵、'O'は王将、'G'は玉将、'K'は金将、'S'は銀将、'A'は角行、'H'は飛車、'T'はと金、'Z'は成銀、'R'は竜王(成飛車)、'M'は竜馬(成角行)を表す。
//'U'は上向き、'D'は下向きを表す。
//possessedは先頭から探索して一度'x'にぶつかったらそれ以降は意味のあるコマが出ないように実装される(則ち、意味のあるコマは先頭に連続してしか出現しない)はず。
char board[5][5][2]={
    {{'H', 'D'},{'A', 'D'},{'S','D'},{'K', 'D'},{'G', 'D'}},
    {{'x', 'x'},{'x', 'x'},{'x', 'x'},{'x', 'x'},{'F', 'D'}},
    {{'x', 'x'},{'x', 'x'},{'x', 'x'},{'x', 'x'},{'x', 'x'}},
    {{'F', 'U'},{'x', 'x'},{'x', 'x'},{'x', 'x'},{'x', 'x'}},
    {{'O', 'U'},{'K', 'U'},{'S','U'},{'A','U'},{'H','U'}}
};
char possessed[2][12]={{'x','x','x','x','x','x','x','x','x','x','x','x'},
    {'x','x','x','x','x','x','x','x','x','x','x','x'}
};
/*char board[5][5][2]={
    {{'S', 'U'},{'x', 'x'},{'x','x'},{'x', 'x'},{'A', 'U'}},
    {{'x', 'x'},{'x', 'x'},{'x', 'x'},{'H', 'D'},{'x', 'x'}},
    {{'O', 'U'},{'x', 'x'},{'x', 'x'},{'H', 'D'},{'x', 'x'}},
    {{'x', 'x'},{'x', 'x'},{'G', 'D'},{'x', 'x'},{'x', 'x'}},
    {{'K', 'D'},{'T', 'D'},{'S','D'},{'x','x'},{'K','U'}}
};
char possessed[2][12]={{'F','x','x','x','x','x','x','x','x','x','x','x'},
    {'A','x','x','x','x','x','x','x','x','x','x','x'}
}; for_debug*/
//これまでの手数(先手後手合わせて)
int Num_Moves=0;


typedef struct AVL_TREE_node{
    char key[6][5][2];
    int value;
    int height, balance;
    struct AVL_TREE_node *left, *right;
} AVL_TREE_Node;
//プロトタイプ宣言
int isOte(int side);
int is_Movable(char koma_name, int sy, int sx, int gy, int gx, int side);
void AVL_TREE_init_node(AVL_TREE_Node* n);
void AVL_TREE_bal_cal(AVL_TREE_Node* n);
void AVL_TREE_set_data(AVL_TREE_Node* n, char key[6][5][2], int value);
int AVL_TREE_search(AVL_TREE_Node* n, char key[6][5][2]);
AVL_TREE_Node* AVL_TREE_insert(AVL_TREE_Node* n, char key[6][5][2], int value, int balancing);
AVL_TREE_Node* AVL_TREE_balancize(AVL_TREE_Node* n);
AVL_TREE_Node* AVL_TREE_rotate(AVL_TREE_Node* n, int isRight);
int Check_If_Permitted_Move(char* input, int side);
int isTsumi(int side);

//これまで通った盤面を保持するためのAVL木
AVL_TREE_Node* Board_Memorizing_AVL_Tree[2]={NULL, NULL};


//盤面表示用にアルファベット一文字(HやFなど)の表記をよく見る漢字一文字(歩や角など)の「文字列」に変換します(漢字は一般に多bitなのでcharでは返せない)
char* Convert2Kanji(char code){
    switch(code){
        case 'F': return "歩";
        case 'O': return "王";
        case 'G': return "玉";
        case 'K': return "金";
        case 'S': return "銀";
        case 'A': return "角";
        case 'H': return "飛";
        case 'T': return "と";
        case 'Z': return "全";
        case 'R': return "龍";
        case 'M': return "馬";
        default:
            printf("\nCode error: invalid code: %c\n", code);//for debug
            exit(1);
            return "??";
    }
}

//成っているコマであるかを返す
int isNari(char koma_name){
    switch(koma_name){
        case 'T': return 1;
        case 'R': return 1;
        case 'M': return 1;
        case 'Z': return 1;
        default: return 0;
    }
}

//成った後のコマの名前を返す関数
char Process_Nari(char before_Nari){
    switch(before_Nari){
        case 'S': return 'Z';
        case 'H': return 'R';
        case 'A': return 'M';
        case 'F': return 'T';
        default: return '?';
    }
}

//千日手の処理のために現在の盤面が何回出たかを返す。
//update==1のときはその値を更新し、0のときは更新しない。
int Process_Sen_Nichi_Te(int update, int side){
    char board_possessed_merged[6][5][2];
    int i, j;
    for(i=0; i<5; i++) for(j=0; j<5; j++){
        board_possessed_merged[i][j][0]=board[i][j][0];
        board_possessed_merged[i][j][1]=board[i][j][1];
    }
    for(i=0; i<2; i++) for(j=0; j<5; j++){
        board_possessed_merged[5][j][i]='0';
        board_possessed_merged[5][j][i]='0';
    }
    for(i=0; i<2; i++) for(j=0; j<12; j++){
        switch(possessed[i][j]){
            case 'F':
                board_possessed_merged[5][0][i]++;
                break;
            case 'K':
                board_possessed_merged[5][1][i]++;
                break;
            case 'S':
                board_possessed_merged[5][2][i]++;
                break;
            case 'A':
                board_possessed_merged[5][3][i]++;
                break;
            case 'H':
                board_possessed_merged[5][4][i]++;
                break;
            default:
                break;
        }
    }

    int Current_Num=AVL_TREE_search(Board_Memorizing_AVL_Tree[side], board_possessed_merged);
    if(Current_Num==-1) Current_Num=0;
    if(update){
        Current_Num++;
        Board_Memorizing_AVL_Tree[side]=AVL_TREE_insert(Board_Memorizing_AVL_Tree[side], board_possessed_merged, Current_Num,1);
    }
    return Current_Num;
}

//駒をなっていない状態に戻す
char Inverse_Process_Nari(char after_Nari){
    switch(after_Nari){
        case 'Z': return 'S';
        case 'R': return 'H';
        case 'M': return 'A';
        case 'T': return 'F';
        default: return after_Nari;
    }
}

void Display_Board(int side){
    printf("\n\n盤面:同じ盤面は%d回目\n\n後手: %s %s %s\n持ち駒(後手):", Process_Sen_Nichi_Te(0,side),(isOte(1) ? "王手している" : "王手していない") , (isOte(0) ? "王手されている" : "王手されていない"), (side ? (isTsumi(side) ? "詰んでいる" : "詰んでいない") : ""));
    int i=0;
    for(i=0; i<12; i++){
        if(possessed[1][i]=='x'){
            printf("\n------------------------------------\n");
            break;
        }
        else printf("%s　", Convert2Kanji(possessed[1][i]));
    }
    int j=0;
    printf("　 A　　B　　C　　D　　E\n\n");
    for(i=0; i<5; i++){
        printf("%d　", 5-i);
        for(j=0; j<5; j++){
            if(board[i][j][0]=='x')
                    printf("　 　");
            else{
                if(board[i][j][1]=='U')
                    printf("%s↑　", Convert2Kanji(board[i][j][0]));
                else
                    printf("%s↓　", Convert2Kanji(board[i][j][0]));
            }
        }
        printf("\n　\n");
    }
    printf("------------------------------------\n持ち駒(先手):");
    for(i=0; i<12; i++){
        if(possessed[0][i]=='x'){
            printf("\n先手: %s　%s %s\n\n", (isOte(0) ? "王手している" : "王手していない") , (isOte(1) ? "王手されている" : "王手されていない"), (side ? "" : (isTsumi(side) ? "詰んでいる" : "詰んでいない")));
            break;
        }
        else printf("%s　", Convert2Kanji(possessed[0][i]));
    }
    if(side==0) printf("先手の手番:\n");
    else printf("後手の手番:\n");
}

//命令の列が駒を打つ命令かどうかを判定する関数
int is_Put_Instruction(char* instruction){
    if(instruction[2]>='1' && instruction[2]<='5') return 0;
    else return 1;
}

//命令の列が成ることを要求しているかを判定する関数
int intended_Nari(char* instruction){
    if(instruction[4]=='N') return 1;
    else return 0;
}

//side側が王手をしているかを判定する関数
//王や玉がいない盤面の場合seg-faultせず-1を返すように変更
int isOte(int side){
    int i, j, gx=-1, gy;

    for(i=0; i<5; i++){
        for(j=0; j<5; j++){
            if(side==0 && board[i][j][0]=='G'){
                gx=j; gy=i;
                break;
            }
            if(side==1 && board[i][j][0]=='O'){
                gx=j; gy=i;
                break;
            }
        }
    }
    if(gx==-1){ return -1;}
    for(i=0; i<5; i++){
        for(j=0; j<5; j++){
            
            //for_debug
            //if(i==3 && j==2){
            //    printf("\nFOR_DEBUG in isOte()$ %d %c %d %d %d %d %d", is_Movable(board[i][j][0], i, j, gy, gx, side), board[i][j][0], i, j, gy, gx, side);
            //}
            
            if(side==0 && board[i][j][1]=='U'){
                if(is_Movable(board[i][j][0], i, j, gy, gx, side)) return 1;
            }
            if(side==1 && board[i][j][1]=='D'){
                if(is_Movable(board[i][j][0], i, j, gy, gx, side)) return 1;
            }
        }
    }
    return 0;
}

//side側が打ったコマが二歩かどうかを判定する関数
int is_Nifu(char* input, int side){
    int i, num=0;
    int column_num=input[1]-'A';
    for(i=0; i<5; i++){
        if(side==0 && board[i][column_num][0]=='F' && board[i][column_num][1]=='U') return 1;
        if(side==1 && board[i][column_num][0]=='F' && board[i][column_num][1]=='D') return 1;
    }
    return 0;
}

//それ以上動けないようにこまをおいた場合
int Unmovable_Koma_Put(char* input, int side){
    if(side==0 && input[0]=='5') return 1;
    if(side==1 && input[0]=='1') return 1;
    return 0;
}

//それ以上動けないようにこまを動かした場合
int Unmovable_Koma_Moved(char* input, int side){
    if(side==0 && input[2]=='5') return 1;
    if(side==1 && input[2]=='1') return 1;
    return 0;
}


//こまを打つ際に正常に判定できていなかったため修正
//千日手処理も正常に判定できていなかったため修正
//side側が詰んでいるかを判定する関数
int isTsumi(int side){
    int Remove_Sen_Nichite=1; //千日手により負ける場合を詰みとみなすか
    if(!isOte(1-side)) return 0;
    if(isOte(side)) return 0;
    int i, j, Tsumiflag=1;
    //駒を動かす場合
    for(i=0; i<5; i++) for(j=0; j<5; j++){
        if(side==0 && board[i][j][1]!='U') continue;
        if(side==1 && board[i][j][1]!='D') continue;
        int k, l;
        for(k=0; k<5; k++) for(l=0; l<5; l++){
            if(!is_Movable(board[i][j][0], i, j, k, l, side)) continue;
            char order[10]="5A5A";
            order[0]-=i;
            order[1]+=j;
            order[2]-=k;
            order[3]+=l;
            //成らない場合
            if(Check_If_Permitted_Move(order, side)){
                int koma_got=-1, Nari_got=0;
                if(board[k][l][0]!='x'){
                    int m;
                    if(isNari(board[k][l][0])){
                        board[k][l][0]=Inverse_Process_Nari(board[k][l][0]);
                        Nari_got=1;
                    }
                    for(m=0; m<12; m++){
                        if(possessed[side][m]=='x'){
                            possessed[side][m]=board[k][l][0];
                            koma_got=m;
                            break;
                        }
                    }
                }
                board[k][l][0]=board[i][j][0];
                board[k][l][1]=board[i][j][1];
                board[i][j][0]='x';
                board[i][j][1]='x';

                if(!isOte(1-side)) Tsumiflag=0;
                
                //for_debug
                //if(!Tsumiflag){
                //    printf("FOR_DEBUG in isTsumi:%d\n", isOte(1-side));
                //    Display_Board(side);
                //}
                
                if(Process_Sen_Nichi_Te(0,1-side)+1>=4){
                    //とった手が千日手
                    if(Remove_Sen_Nichite){
                        //自分が先手なら千日手で負けてしまう
                        if(side==0){Tsumiflag=1;}
                    }
                    //王手千日手
                    if(isOte(side)){Tsumiflag=1;}
                }
                
                board[i][j][0]=board[k][l][0];
                board[i][j][1]=board[k][l][1];
                if(koma_got==-1){
                    board[k][l][0]='x';
                    board[k][l][1]='x';
                }
                else{
                    board[k][l][0]=possessed[side][koma_got];
                    if(Nari_got) board[k][l][0]=Process_Nari(board[k][l][0]);
                    board[k][l][1]=(side ? 'U' : 'D');
                    possessed[side][koma_got]='x';
                }
                
                if(!Tsumiflag){
                    return 0;
                }
            }
            //成る場合
            order[4]='N';
            if(Check_If_Permitted_Move(order, side)){
                int koma_got=-1, Nari_got=0;
                if(board[k][l][0]!='x'){
                    int m;
                    if(isNari(board[k][l][0])){
                        board[k][l][0]=Inverse_Process_Nari(board[k][l][0]);
                        Nari_got=1;
                    }
                    for(m=0; m<12; m++){
                        if(possessed[side][m]=='x'){
                            possessed[side][m]=board[k][l][0];
                            koma_got=m;
                            break;
                        }
                    }
                }
                board[k][l][0]=Process_Nari(board[i][j][0]);
                board[k][l][1]=board[i][j][1];
                board[i][j][0]='x';
                board[i][j][1]='x';
                
                if(!isOte(1-side)) Tsumiflag=0;
                
                if(Process_Sen_Nichi_Te(0,1-side)+1>=4){
                    //とった手が千日手
                    if(Remove_Sen_Nichite){
                        //自分が先手なら千日手で負けてしまう
                        if(side==0){Tsumiflag=1;}
                    }
                    //王手千日手
                    if(isOte(side)){Tsumiflag=1;}
                }
                
                board[i][j][0]=Inverse_Process_Nari(board[k][l][0]);
                board[i][j][1]=board[k][l][1];
                if(koma_got==-1){
                    board[k][l][0]='x';
                    board[k][l][1]='x';
                }
                else{
                    board[k][l][0]=possessed[side][koma_got];
                    if(Nari_got) board[k][l][0]=Process_Nari(board[k][l][0]);
                    board[k][l][1]=(side ? 'U' : 'D');
                    possessed[side][koma_got]='x';
                }
                if(!Tsumiflag){
                    return 0;
                }
            }
        }
    }
    //駒を置く場合
    int IfPossess[5]={-1,-1,-1,-1,-1};
    int Break_Flag=-1;
    for(j=0; j<12; j++){
        switch(possessed[side][j]){
            case 'F':
                IfPossess[4]=j;
                break;
            case 'K':
                IfPossess[3]=j;
                break;
            case 'S':
                IfPossess[2]=j;
                break;
            case 'A':
                IfPossess[1]=j;
                break;
            case 'H':
                IfPossess[0]=j;
                break;
            default:
                Break_Flag=j;
                break;
        }
        if(Break_Flag!=-1) break;
    }
    int k;
    for(k=0; k<5; k++){
        if(IfPossess[k]==-1) continue;
        for(i=0; i<5; i++) for(j=0; j<5; j++){
            char order[10]="5A  ";
            order[0]-=i;
            order[1]+=j;
            char koma_name='x';
            switch(k){
                case 0:
                    order[2]='H'; order[3]='I'; koma_name='H'; break;
                case 1:
                    order[2]='K'; order[3]='K'; koma_name='A'; break;
                case 2:
                    order[2]='G'; order[3]='I'; koma_name='S'; break;
                case 3:
                    order[2]='K'; order[3]='I'; koma_name='K'; break;
                case 4:
                    order[2]='F'; order[3]='U'; koma_name='F'; break;
                default: break;
            }
            if(Check_If_Permitted_Move(order, side)){
                board[i][j][0]=koma_name;
                board[i][j][1]=(side ? 'D' : 'U');
                if(Break_Flag>1 && IfPossess[k]+1<Break_Flag){
                    possessed[side][IfPossess[k]]=possessed[side][Break_Flag-1];
                    possessed[side][Break_Flag-1]='x';
                }
                else possessed[side][IfPossess[k]]='x';
                
                if(isOte(1-side)) Tsumiflag=0;
                
                if(koma_name=='F'){
                    if(isTsumi(1-side)){
                        //打ち歩詰めの場合
                        Tsumiflag=1;
                    }
                }
                
                if(Process_Sen_Nichi_Te(0,1-side)+1>=4){
                    //とった手が千日手
                    if(Remove_Sen_Nichite){
                        //自分が先手なら千日手で負けてしまう
                        if(side==0){Tsumiflag=1;}
                    }
                    //王手千日手
                    if(isOte(side)){Tsumiflag=1;}
                }
                
                if(Break_Flag>1 && IfPossess[k]+1<Break_Flag){
                    possessed[side][Break_Flag-1]=possessed[side][IfPossess[k]];
                    possessed[side][IfPossess[k]]=board[i][j][0];
                }
                else possessed[side][IfPossess[k]]=board[i][j][0];
                board[i][j][0]='x';
                board[i][j][1]='x';
                
                if(!Tsumiflag){
                    return 0;
                }
            }
        }
    }
    return 1;
}

//side側が歩詰めしようとしているかを判定する関数
int isFudume(char* input, int side){
    int j, Fu_index, max_index, Flag=0;
    for(j=0; j<12; j++){
        if(possessed[side][j]=='F'){
            Fu_index=j;
        }
        if(possessed[side][j]=='x'){
            max_index=j;
            break;
        }
    }
    if(max_index>1 && Fu_index+1<max_index){
        possessed[side][Fu_index]=possessed[side][max_index-1];
        possessed[side][max_index-1]='x';
    }
    else possessed[side][Fu_index]='x';
    int x=4-input[0]+'1', y=input[1]-'A';
    board[x][y][0]='F';
    board[x][y][1]=(side ? 'D' : 'U');
    
    if(isTsumi(1-side)) Flag=1;
    
    if(max_index>1 && Fu_index+1<max_index){
        possessed[side][max_index-1]=possessed[side][Fu_index];
        possessed[side][Fu_index]='F';
    }
    else possessed[side][Fu_index]='F';
    board[x][y][0]='x';
    board[x][y][1]='x';
    
    return Flag;
}

//駒が(動きとして)動けるかどうかを判定する関数
int is_Movable(char koma_name, int sy, int sx, int gy, int gx, int side){
    switch(koma_name){
        case 'F': //歩兵
            if(side==0 && ((sx!=gx) || (gy!=sy-1))) return 0;
            if(side==1 && ((sx!=gx) || (gy!=sy+1))) return 0;
            if(side==0 && board[gy][gx][1]=='U') return 0;
            if(side==1 && board[gy][gx][1]=='D') return 0;
            break;
            
        case 'O': //王将
            ;
        case 'G': //玉将
            if((sy-gy)*(sy-gy)>1 || (sx-gx)*(sx-gx)>1 || (sx==gx && sy==gy)) return 0;
            if(side==0 && board[gy][gx][1]=='U') return 0;
            if(side==1 && board[gy][gx][1]=='D') return 0;
            break;
            
        case 'S': //銀将
            if((sy-gy)*(sy-gy)>1 || (sx-gx)*(sx-gx)>1 || (sx==gx && sy==gy)) return 0;
            if(side==0 && (sy==gy || (sx==gx && gy!=sy-1))) return 0;
            if(side==1 && (sy==gy || (sx==gx && gy!=sy+1))) return 0;
            if(side==0 && board[gy][gx][1]=='U') return 0;
            if(side==1 && board[gy][gx][1]=='D') return 0;
            break;
            
        case 'K'://金将
            ;//何もしない
        case 'T'://と金
            ;//何もしない
        case 'Z'://成銀
            if((sy-gy)*(sy-gy)>1 || (sx-gx)*(sx-gx)>1 || (sx==gx && sy==gy)) return 0;
            if(side==0 && (sx!=gx && gy==sy+1)) return 0;
            if(side==1 && (sx!=gx && gy==sy-1)) return 0;
            if(side==0 && board[gy][gx][1]=='U') return 0;
            if(side==1 && board[gy][gx][1]=='D') return 0;
            break;
            
        case 'R'://龍王(成飛車)
            if((sy-gy)*(sy-gy)>1 || (sx-gx)*(sx-gx)>1 || (sx==gx && sy==gy)) ;
            else{
                if(side==0 && board[gy][gx][1]=='U') return 0;
                if(side==1 && board[gy][gx][1]=='D') return 0;
                return 1;
            }
            if(sx==gx && sy==gy) return 0;
        case 'H': //飛車
            if(sx!=gx && sy!=gy) return 0;
            if(side==0 && board[gy][gx][1]=='U') return 0;
            if(side==1 && board[gy][gx][1]=='D') return 0;
            if(sx==gx){
                int Minim=(sy>gy ? gy : sy)+1, Maxim=(sy<gy ? gy : sy);
                for(; Minim<Maxim; Minim++) if(board[Minim][sx][0]!='x') return 0;
            }
            else{
                int Minim=(sx>gx ? gx : sx)+1, Maxim=(sx<gx ? gx : sx);
                for(; Minim<Maxim; Minim++) if(board[sy][Minim][0]!='x') return 0;
            }
            break;
            
        case 'M'://龍馬(成角行)
            if((sy-gy)*(sy-gy)>1 || (sx-gx)*(sx-gx)>1 || (sx==gx && sy==gy)) ;
            else{
                if(side==0 && board[gy][gx][1]=='U') return 0;
                if(side==1 && board[gy][gx][1]=='D') return 0;
                return 1;
            }
            if(sx==gx && sy==gy) return 0;
        case 'A': //角行
            if(sx-gx!=sy-gy && sx-gx!=gy-sy) return 0;
            if(side==0 && board[gy][gx][1]=='U') return 0;
            if(side==1 && board[gy][gx][1]=='D') return 0;
            if(sx-gx==sy-gy){
                int X=(sx>gx ? gx : sx)+1, Y=(sy>gy ? gy : sy)+1, MaximX=(sx<gx ? gx : sx);
                for(; X<MaximX; X++, Y++) if(board[Y][X][0]!='x') return 0;
            }
            else{
                int X=(sx>gx ? gx : sx)+1, Y=(sy<gy ? gy : sy)-1, MaximX=(sx<gx ? gx : sx);
                for(; X<MaximX; X++, Y--) if(board[Y][X][0]!='x') return 0;
            }
            break;
            
        default:
            printf("\nCode error: invalid code: %c\n", koma_name);//for debug
            return 0;
    }
    return 1;
}

//命令列がゲームとして許容されるかを判定する関数(王手千日手と歩詰めのみはmain関数中及びPlayer_Process/AI_Processで判定しているため判定されない)
int Check_If_Permitted_Move(char* input, int side){
    if(is_Put_Instruction(input)){ //打つ場合
        char koma_side, koma_name;
        if(input[2]=='H' && input[3]=='I') koma_name='H';
        else if(input[2]=='K' && input[3]=='K') koma_name='A';
        else if(input[2]=='K' && input[3]=='I') koma_name='K';
        else if(input[2]=='G' && input[3]=='I') koma_name='S';
        else if(input[2]=='F' && input[3]=='U') koma_name='F';
        else return 0;
        int i=0, found_index=-1;
        for(i=0; i<12; i++){
            if(possessed[side][i]=='x'){
                break;
            }
            if(possessed[side][i]==koma_name){
                found_index=i;
            }
        }
        if(found_index==-1){
            return 0;
        }
        if(board[4-input[0]+'1'][input[1]-'A'][0]=='x'){
            if(koma_name=='F'){
                if(is_Nifu(input, side)) return 0;
                if(Unmovable_Koma_Put(input, side)) return 0;
            }
            else return 1;
        }
        else return 0;
    }
    else{
        if((side==0 && board[4-input[0]+'1'][input[1]-'A'][1]=='U') || (side==1 && board[4-input[0]+'1'][input[1]-'A'][1]=='D') ){
            if(is_Movable(board[4-input[0]+'1'][input[1]-'A'][0], 4-input[0]+'1', input[1]-'A', 4-input[2]+'1', input[3]-'A', side)){
                ;
            }
            else return 0; //駒としてそこに動けない場合
        }
        else return 0; //自分のコマでなかったり駒がなかったりする場合
        if(intended_Nari(input)){
            int koma_name=board[4-input[0]+'1'][input[1]-'A'][0];
            if(Process_Nari(koma_name)=='?') return 0;
            if(side==1 && 4-input[0]+'1'<4 && 4-input[2]+'1'<4) return 0;
            if(side==0 && 4-input[0]+'1'>0 && 4-input[2]+'1'>0) return 0;
        }
        else{
            if(board[4-input[0]+'1'][input[1]-'A'][0]=='F')
                if(Unmovable_Koma_Moved(input, side)) return 0;
        }
        return 1;
    }
    return -1;//for debug(ここは通らないはず)
}

//ユーザの手番の処理
void Player_Process(int side){
    char input[10];
    scanf("%s", input);
    if(Check_If_Permitted_Move(input, side)){
        //歩詰めの場合
        if(input[2]=='F' && input[3]=='U'){
            if(isFudume(input, side)){
                if(First_Player<=1) printf("You Lose\n");
                else printf("%s負け\n", (side ? "後手" : "先手"));
                exit(0);
            }
        }
        
        if(is_Put_Instruction(input)){
            char koma_side, koma_name;
            if(input[2]=='H' && input[3]=='I') koma_name='H';
            if(input[2]=='K' && input[3]=='K') koma_name='A';
            if(input[2]=='K' && input[3]=='I') koma_name='K';
            if(input[2]=='G' && input[3]=='I') koma_name='S';
            if(input[2]=='F' && input[3]=='U') koma_name='F';
            int i=0, found_index=-1, max_index=-1;
            for(i=0; i<12; i++){
                if(possessed[side][i]=='x'){
                    break;
                }
                max_index++;
                if(possessed[side][i]==koma_name){
                    found_index=i;
                }
            }
            if(found_index==-1){
                printf("Error: No Koma Available to Put\n");
                exit(0);
            }
            if(max_index>0 && found_index!=max_index){
                possessed[side][found_index]=possessed[side][max_index];
                possessed[side][max_index]='x';
            }
            else possessed[side][found_index]='x';
            if(side==0) koma_side='U';
            else koma_side='D';
            board[4-input[0]+'1'][input[1]-'A'][0]=koma_name;
            board[4-input[0]+'1'][input[1]-'A'][1]=koma_side;
        }
        else{
            char koma_name=board[4-input[0]+'1'][input[1]-'A'][0];
            char koma_side=board[4-input[0]+'1'][input[1]-'A'][1];
            if(board[4-input[2]+'1'][input[3]-'A'][0]!='x'){
                int i=0;
                for(i=0; i<12; i++){
                    if(possessed[side][i]=='x'){
                        possessed[side][i]=Inverse_Process_Nari(board[4-input[2]+'1'][input[3]-'A'][0]);//駒を取る
                        break;
                    }
                }
            }
            board[4-input[0]+'1'][input[1]-'A'][0]='x';
            board[4-input[0]+'1'][input[1]-'A'][1]='x';
            if(intended_Nari(input)){
                koma_name=Process_Nari(koma_name);
            }
            board[4-input[2]+'1'][input[3]-'A'][0]=koma_name;
            board[4-input[2]+'1'][input[3]-'A'][1]=koma_side;
        }
    }
    else{
        if(First_Player<=1) printf("You Lose\n");
        else printf("%s負け\n", (side ? "後手" : "先手"));
        exit(0);
    }
    return;
}

//AIの手番の処理
void AI_Process(int side){
    char* input=ALPHA_BETA_FUNC_NAME(side);
    printf("%s\n", input);
    if(Check_If_Permitted_Move(input, side)){
        //歩詰めの場合
        if(input[2]=='F' && input[3]=='U'){
            if(isFudume(input, side)){
                printf("\nError: Invalid Move by AI\n");
            }
        }
        
        if(is_Put_Instruction(input)){
            char koma_side, koma_name;
            if(input[2]=='H' && input[3]=='I') koma_name='H';
            if(input[2]=='K' && input[3]=='K') koma_name='A';
            if(input[2]=='K' && input[3]=='I') koma_name='K';
            if(input[2]=='G' && input[3]=='I') koma_name='S';
            if(input[2]=='F' && input[3]=='U') koma_name='F';
            int i=0, found_index=-1, max_index=-1;
            for(i=0; i<12; i++){
                if(possessed[side][i]=='x'){
                    break;
                }
                max_index++;
                if(possessed[side][i]==koma_name){
                    found_index=i;
                }
            }
            if(found_index==-1){
                printf("Error: No Koma Available to Put\n");
                exit(0);
            }
            if(max_index>0 && found_index!=max_index){
                possessed[side][found_index]=possessed[side][max_index];
                possessed[side][max_index]='x';
            }
            else possessed[side][found_index]='x';
            if(side==0) koma_side='U';
            else koma_side='D';
            board[4-input[0]+'1'][input[1]-'A'][0]=koma_name;
            board[4-input[0]+'1'][input[1]-'A'][1]=koma_side;
        }
        else{
            char koma_name=board[4-input[0]+'1'][input[1]-'A'][0];
            char koma_side=board[4-input[0]+'1'][input[1]-'A'][1];
            if(board[4-input[2]+'1'][input[3]-'A'][0]!='x'){
                int i=0;
                for(i=0; i<12; i++){
                    if(possessed[side][i]=='x'){
                        possessed[side][i]=Inverse_Process_Nari(board[4-input[2]+'1'][input[3]-'A'][0]);//駒を取る
                        break;
                    }
                }
            }
            board[4-input[0]+'1'][input[1]-'A'][0]='x';
            board[4-input[0]+'1'][input[1]-'A'][1]='x';
            if(intended_Nari(input)){
                koma_name=Process_Nari(koma_name);
            }
            board[4-input[2]+'1'][input[3]-'A'][0]=koma_name;
            board[4-input[2]+'1'][input[3]-'A'][1]=koma_side;
        }
    }
    else{
        printf("\nError: Invalid Move by AI\n");
    }
    return;
}

//メイン関数
int main(int argc, char** argv){
    //今の盤面になった回数を保持する変数
    if(argc!=2){
        printf("Invalid Parameter\n");
        exit(1);
    }
    First_Player=argv[1][0]-'0';
    int Num_This_Board=Process_Sen_Nichi_Te(1, 0);
    while(1){
        //先手の手番
        if(Display_Board_Enabled) Display_Board(0);
        if(First_Player==0 || First_Player==2){
            Player_Process(0);
            if(isOte(1)){
                if(Display_Board_Enabled) Display_Board(1);
                if(First_Player==0) printf("You Lose\n");
                else printf("先手負け\n");
                return 0;
            }
        }
        else{
            AI_Process(0);
            if(isOte(1)){
                if(Display_Board_Enabled) Display_Board(1);
                if(First_Player==1) printf("You Win\n");
                else printf("先手勝ち\n");
                return 0;
            }
        }
        Num_This_Board=Process_Sen_Nichi_Te(1, 1);
        if(isTsumi(1)){
            //打った結果相手が詰んだ
            if(Display_Board_Enabled) Display_Board(1);
            if(First_Player==0) printf("You Win\n");
            else if(First_Player==1) printf("You Lose\n");
            else printf("先手勝ち\n");
            return 0;
        }
        if(Num_This_Board>=4){
            if(Display_Board_Enabled) Display_Board(1);
            //王手千日手は王手をかけている側が負け
            if(isOte(0)){
                if(First_Player==0) printf("You Lose\n");
                else if(First_Player==1) printf("You Win\n");
                else printf("先手負け\n");
                return 0;
            }
            //通常の千日手は後手勝ち
            if(First_Player==0) printf("You Lose\n");
            else if(First_Player==1) printf("You Win\n");
            else printf("後手勝ち\n");
            return 0;
        }
        Num_Moves++;
        if(Num_Moves>=150){
            printf("Draw\n");
            return 0;
        }
        //後手の手番
        if(Display_Board_Enabled) Display_Board(1);
        if(First_Player==1 || First_Player==2){
            Player_Process(1);
            if(isOte(0)){
                if(Display_Board_Enabled) Display_Board(0);
                if(First_Player==1) printf("You Lose\n");
                else printf("後手負け\n");
                return 0;
            }
        }
        else{
            AI_Process(1);
            if(isOte(0)){
                if(Display_Board_Enabled) Display_Board(0);
                if(First_Player==0) printf("You Win\n");
                else printf("先手勝ち\n");
                return 0;
            }
        }
        Num_This_Board=Process_Sen_Nichi_Te(1, 0);
        if(isTsumi(0)){
            //打った結果相手が詰んだ
            if(Display_Board_Enabled) Display_Board(0);
            if(First_Player==0) printf("You Lose\n");
            else if(First_Player==1) printf("You Win\n");
            else printf("後手勝ち\n");
            return 0;
        }
        if(Num_This_Board>=4){
            if(Display_Board_Enabled) Display_Board(0);
            //王手千日手は王手をかけている側が負け
            if(isOte(1)){
                if(First_Player==0) printf("You Win\n");
                else if(First_Player==1) printf("You Lose\n");
                else printf("後手負け\n");
                return 0;
            }
            //通常の千日手は後手勝ち
            if(First_Player==0) printf("You Lose\n");
            else if(First_Player==1)  printf("You Win\n");
            else printf("後手勝ち\n");
            return 0;
        }
        Num_Moves++;
        if(Num_Moves>=150){
            printf("Draw\n");
            return 0;
        }
        //break; //for debug
    }
    return 0;
}


//以降はAlpha-Beta法関連
//文字列と駒の始点と終点と成るかどうか(0:成らない、1:成る)を与えるとその文字列を対応する命令に変換する関数
//search_Next関数内で使った
void return_move_instruction(char* inst, int sy, int sx, int gy, int gx, int is_nari){
    inst[0] = '0' + 5 - sy;
    inst[1] = 'A' + sx;
    inst[2] = '0' + 5 - gy;
    inst[3] = 'A' + gx;
    if(is_nari == 0){
        inst[4] = '\0';
    }
    else{
        inst[4] = 'N';
        inst[5] = '\0';
    }
}

//文字列と駒を置きたい場所と駒の種類を与えるとその文字列を対応する命令に変換する関数
//search_Next関数内で使った
void return_put_instruction(char* inst, int y, int x, char koma_name){
    inst[0] = '0' + 5 - y;
    inst[1] = 'A' + x;
    switch(koma_name){
        case 'H':
            inst[2] = 'H';
            inst[3] = 'I';
            break;
        case 'A':
            inst[2] = 'K';
            inst[3] = 'K';
            break;
        case 'K':
            inst[2] = 'K';
            inst[3] = 'I';
            break;
        case 'S':
            inst[2] = 'G';
            inst[3] = 'I';
            break;
        case 'F':
            inst[2] = 'F';
            inst[3] = 'U';
            break;
    }
    inst[4] = '\0';
}

//引数の盤面においてinputの手が反則かどうかを判断する
//0:反則じゃない　1:反則
int Check_Faul(char check_board[5][5][2], char check_possessed[2][12], int side, char* input){
    int ret = 0;
    char now_board[5][5][2];
    char now_possessed[2][12];
    //一旦グローバル変数を変更
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            for(int k = 0; k < 2; k++){
                now_board[i][j][k] = board[i][j][k];
                board[i][j][k] = check_board[i][j][k];
            }
        }
    }
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 12; j++){
            now_possessed[i][j] = possessed[i][j];
            possessed[i][j] = check_possessed[i][j];
        }
    }

    if(is_Nifu(input, side)) ret = 1;
    else if(Process_Sen_Nichi_Te(0, side) >= 4) ret = 1;
    else if(input[2] == 'F' && input[3] == 'U'){
        if(isFudume(input, side)) ret = 1;
    }

    //グローバル変数を元に戻す
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            for(int k = 0; k < 2; k++){
                board[i][j][k] = now_board[i][j][k];
            }
        }
    }
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 12; j++){
            possessed[i][j] = now_possessed[i][j];
        }
    }
    return ret;
}

//盤面の状態(board,possessed,side)から、次うてる手の数を返す関数
//次うてる手の命令は引数に与えた配列の中に入れられる
//まだ反則手(二歩、打ち歩詰め等)は判定できていない
int search_Next(char board[5][5][2], char possessed[2][12], int side, char instruction_list[500][6]){
    char side_char = ((side == 0)? 'U' : 'D');//side_charは先手ならU、後手ならD
    int instruction_index = 0;
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            if(board[i][j][1] == side_char){//自分の駒の時
                switch(board[i][j][0]){//駒の種類によって場合分け
                    case 'F'://歩
                        if(side == 0){
                            if(i > 0 && board[i - 1][j][1] != side_char){
                                if(i == 1){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i-1, j, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, i-1, j, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                            }
                        }
                        else{
                            if(i < 4 && board[i + 1][j][1] != side_char){
                                if(i == 3){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+1, j, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+1, j, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                            }
                        }
                        break;
                    case 'O'://王(玉と同じ)
                        ;
                    case 'G'://玉
                        for(int k = -1; k <= 1; k++){
                            for(int l = -1; l <= 1; l++){
                                if(k == 0 && l == 0) continue;
                                if(i + k < 0 || i + k > 4 || j + l < 0 || j + l > 4) continue;
                                if(board[i+k][j+l][1] != side_char){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+k, j+l, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                            }
                        }
                        break;
                    case 'S'://銀
                        if(side == 0){
                            if(i > 0){
                                for(int l = -1; l <= 1; l++){
                                    if(j + l < 0 || j + l > 4) continue;
                                    if(board[i - 1][j + l][1] == side_char) continue;
                                    return_move_instruction(instruction_list[instruction_index], i, j, i-1, j+l, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                    if(i - 1 == 0){
                                        return_move_instruction(instruction_list[instruction_index], i, j, i-1, j+l, 1);
                                        if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                            instruction_index++;
                                        }
                                    }
                                }
                            }
                            if(i < 4){
                                for(int l = -1; l <= 1; l++){
                                    if(l == 0) continue;
                                    if(j + l < 0 || j + l > 4) continue;
                                    if(board[i + 1][j + l][1] == side_char) continue;
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+1, j+l, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                    if(i == 0){
                                        return_move_instruction(instruction_list[instruction_index], i, j, i+1, j+l, 1);
                                        if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                            instruction_index++;
                                        }
                                    }
                                }
                            }
                        }
                        else{
                            if(i < 4){
                                for(int l = -1; l <= 1; l++){
                                    if(j + l < 0 || j + l > 4) continue;
                                    if(board[i + 1][j + l][1] == side_char) continue;
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+1, j+l, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                    if(i + 1 == 4){
                                        return_move_instruction(instruction_list[instruction_index], i, j, i+1, j+l, 1);
                                        if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                            instruction_index++;
                                        }
                                    }
                                }
                            }
                            if(i > 0){
                                for(int l = -1; l <= 1; l++){
                                    if(l == 0) continue;
                                    if(j + l < 0 || j + l > 4) continue;
                                    if(board[i - 1][j + l][1] == side_char) continue;
                                    return_move_instruction(instruction_list[instruction_index], i, j, i-1, j+l, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                    if(i == 4){
                                        return_move_instruction(instruction_list[instruction_index], i, j, i-1, j+l, 1);
                                        if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                            instruction_index++;
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    case 'K'://金(成銀と同じ)
                        ;
                    case 'T'://と金(成銀と同じ)
                        ;
                    case 'Z'://成銀
                        if(side == 0){
                            for(int k = -1; k <= 0; k++){
                                for(int l = -1; l <= 1; l++){
                                    if(k == 0 && l == 0) continue;
                                    if(i + k < 0 || i + k > 4 || j + l < 0 || j + l > 4) continue;
                                    if(board[i+k][j+l][1] != side_char){
                                        return_move_instruction(instruction_list[instruction_index], i, j, i+k, j+l, 0);
                                        if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                            instruction_index++;
                                        }
                                    }
                                }
                            }
                            if(i < 4 && board[i+1][j][1] != side_char){
                                return_move_instruction(instruction_list[instruction_index], i, j, i+1, j, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        else{
                            for(int k = 0; k <= 1; k++){
                                for(int l = -1; l <= 1; l++){
                                    if(k == 0 && l == 0) continue;
                                    if(i + k < 0 || i + k > 4 || j + l < 0 || j + l > 4) continue;
                                    if(board[i+k][j+l][1] != side_char){
                                        return_move_instruction(instruction_list[instruction_index], i, j, i+k, j+l, 0);
                                        if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                            instruction_index++;
                                        }
                                    }
                                }
                            }
                            if(i > 0 && board[i-1][j][1] != side_char){
                                return_move_instruction(instruction_list[instruction_index], i, j, i-1, j, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        break;
                    case 'R'://竜
                        for(int k = -1; k <= 1; k++){
                            for(int l = -1; l <= 1; l++){
                                if(k == 0 || l == 0) continue;
                                if(i + k < 0 || i + k > 4 || j + l < 0 || j + l > 4) continue;
                                if(board[i+k][j+l][1] != side_char){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+k, j+l, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                            }
                        }
                    case 'H'://飛車
                        for(int k = i + 1; k < 5; k++){
                            if(board[k][j][1] == side_char){
                                break;
                            }
                            else if(board[k][j][1] != 'x'){
                                if(((side == 0 && i == 0) || (side == 1 && k == 4)) && board[i][j][0] == 'H'){
                                    return_move_instruction(instruction_list[instruction_index], i, j, k, j, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, k, j, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                break;
                            }
                            if(((side == 0 && i == 0) || (side == 1 && k == 4)) && board[i][j][0] == 'H'){
                                return_move_instruction(instruction_list[instruction_index], i, j, k, j, 1);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }else{
                                return_move_instruction(instruction_list[instruction_index], i, j, k, j, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        for(int k = i - 1; k >= 0; k--){
                            if(board[k][j][1] == side_char){
                                break;
                            }
                            else if(board[k][j][1] != 'x'){
                                if(((side == 0 && k == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'H'){
                                    return_move_instruction(instruction_list[instruction_index], i, j, k, j, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, k, j, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                break;
                            }
                            if(((side == 0 && k == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'H'){
                                return_move_instruction(instruction_list[instruction_index], i, j, k, j, 1);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }else{
                                return_move_instruction(instruction_list[instruction_index], i, j, k, j, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        for(int l = j + 1; l < 5; l++){
                            if(board[i][l][1] == side_char){
                                break;
                            }
                            else if(board[i][l][1] != 'x'){
                                if(((side == 0 && i == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'H'){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i, l, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, i, l, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                break;
                            }
                            if(((side == 0 && i == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'H'){
                                return_move_instruction(instruction_list[instruction_index], i, j, i, l, 1);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }else{
                                return_move_instruction(instruction_list[instruction_index], i, j, i, l, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        for(int l = j - 1; l > 0; l--){
                            if(board[i][l][1] == side_char){
                                break;
                            }
                            else if(board[i][l][1] != 'x'){
                                if(((side == 0 && i == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'H'){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i, l, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, i, l, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                break;
                            }
                            if(((side == 0 && i == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'H'){
                                return_move_instruction(instruction_list[instruction_index], i, j, i, l, 1);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }else{
                                return_move_instruction(instruction_list[instruction_index], i, j, i, l, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        break;
                    case 'M'://馬
                        for(int k = -1; k <= 1; k++){
                            for(int l = -1; l <= 1; l++){
                                if(k - l != 1 && k - l != -1) continue;
                                if(i + k < 0 || i + k > 4 || j + l < 0 || j + l > 4) continue;
                                if(board[i+k][j+l][1] != side_char){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+k, j+l, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                            }
                        }
                    case 'A'://角行
                        for(int k = 1; k < 5; k++){
                            if(i + k > 4 || j + k > 4) break;
                            if(board[i+k][j+k][1] == side_char){
                                break;
                            }
                            else if(board[i+k][j+k][1] != 'x'){
                                if(((side == 0 && i == 0) || (side == 1 && i+k == 4)) && board[i][j][0] == 'A'){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+k, j+k, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+k, j+k, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                break;
                            }
                            if(((side == 0 && i == 0) || (side == 1 && i+k == 4)) && board[i][j][0] == 'A'){
                                return_move_instruction(instruction_list[instruction_index], i, j, i+k, j+k, 1);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }else{
                                return_move_instruction(instruction_list[instruction_index], i, j, i+k, j+k, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        for(int k = 1; k < 5; k++){
                            if(i - k < 0 || j + k > 4) break;
                            if(board[i-k][j+k][1] == side_char){
                                break;
                            }
                            else if(board[i-k][j+k][1] != 'x'){
                                if(((side == 0 && i-k == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'A'){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i-k, j+k, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, i-k, j+k, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                break;
                            }
                            if(((side == 0 && i-k == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'A'){
                                return_move_instruction(instruction_list[instruction_index], i, j, i-k, j+k, 1);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }else{
                                return_move_instruction(instruction_list[instruction_index], i, j, i-k, j+k, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        for(int k = 1; k < 5; k++){
                            if(i + k > 4 || j - k < 0) break;
                            if(board[i+k][j-k][1] == side_char){
                                break;
                            }
                            else if(board[i+k][j-k][1] != 'x'){
                                if(((side == 0 && i == 0) || (side == 1 && i+k == 4)) && board[i][j][0] == 'A'){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+k, j-k, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, i+k, j-k, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                break;
                            }
                            if(((side == 0 && i == 0) || (side == 1 && i+k == 4)) && board[i][j][0] == 'A'){
                                return_move_instruction(instruction_list[instruction_index], i, j, i+k, j-k, 1);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }else{
                                return_move_instruction(instruction_list[instruction_index], i, j, i+k, j-k, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        for(int k = 1; k < 5; k++){
                            if(i - k < 0 || j - k < 0) break;
                            if(board[i-k][j-k][1] == side_char){
                                break;
                            }
                            else if(board[i-k][j-k][1] != 'x'){
                                if(((side == 0 && i-k == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'A'){
                                    return_move_instruction(instruction_list[instruction_index], i, j, i-k, j-k, 1);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }else{
                                    return_move_instruction(instruction_list[instruction_index], i, j, i-k, j-k, 0);
                                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                        instruction_index++;
                                    }
                                }
                                break;
                            }
                            if(((side == 0 && i-k == 0) || (side == 1 && i == 4)) && board[i][j][0] == 'A'){
                                return_move_instruction(instruction_list[instruction_index], i, j, i-k, j-k, 1);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }else{
                                return_move_instruction(instruction_list[instruction_index], i, j, i-k, j-k, 0);
                                if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                                    instruction_index++;
                                }
                            }
                        }
                        break;
                }
            }
            else if(board[i][j][1] == 'x'){//空いているマスの時
                for(int k = 0; k < 12; k++){
                    if(possessed[side][k] == 'x') break;
                    if(possessed[side][k] == 'G' || possessed[side][k] == 'O') continue;
                    if(((side == 0 && i == 0) || (side == 1 && i == 4)) && possessed[side][k] == 'F') continue;//歩は相手の方の端には打てない

                    return_put_instruction(instruction_list[instruction_index], i, j, possessed[side][k]);
                    if(!Check_Faul(board, possessed, side, instruction_list[instruction_index])){
                        instruction_index++;
                    }
                }
            }
        }
    }
    return instruction_index;
}


void Move(char board[5][5][2], char possessed[2][12], int side, char* input){
    if(is_Put_Instruction(input)){
        char koma_side, koma_name;
        if(input[2]=='H' && input[3]=='I') koma_name='H';
        if(input[2]=='K' && input[3]=='K') koma_name='A';
        if(input[2]=='K' && input[3]=='I') koma_name='K';
        if(input[2]=='G' && input[3]=='I') koma_name='S';
        if(input[2]=='F' && input[3]=='U') koma_name='F';
        int i=0, found_index=-1, max_index=-1;
        for(i=0; i<12; i++){
            if(possessed[side][i]=='x'){
                break;
            }
            max_index++;
            if(possessed[side][i]==koma_name){
                found_index=i;
            }
        }
        if(found_index==-1){
            printf("Error: No Koma Available to Put\n");
            exit(0);
        }
        if(max_index>0 && found_index!=max_index){
            possessed[side][found_index]=possessed[side][max_index];
            possessed[side][max_index]='x';
        }
        else possessed[side][found_index]='x';
        if(side==0) koma_side='U';
        else koma_side='D';
        board[4-input[0]+'1'][input[1]-'A'][0]=koma_name;
        board[4-input[0]+'1'][input[1]-'A'][1]=koma_side;
    }
    else{
        char koma_name=board[4-input[0]+'1'][input[1]-'A'][0];
        char koma_side=board[4-input[0]+'1'][input[1]-'A'][1];
        if(board[4-input[2]+'1'][input[3]-'A'][0]!='x'){
            int i=0;
            for(i=0; i<12; i++){
                if(possessed[side][i]=='x'){
                    possessed[side][i]=Inverse_Process_Nari(board[4-input[2]+'1'][input[3]-'A'][0]);//駒を取る
                    break;
                }
            }
        }
        board[4-input[0]+'1'][input[1]-'A'][0]='x';
        board[4-input[0]+'1'][input[1]-'A'][1]='x';
        if(intended_Nari(input)){
            koma_name=Process_Nari(koma_name);
        }
        board[4-input[2]+'1'][input[3]-'A'][0]=koma_name;
        board[4-input[2]+'1'][input[3]-'A'][1]=koma_side;
    }
}

int Omomi(char koma){
    switch(koma){
        case 'F':
            return 10;
        case 'O':
            return 100;
        case 'G':
            return 100;
        case 'K':
            return 60;
        case 'S':
            return 40;
        case 'A':
            return 80;
        case 'H':
            return 90;
        case 'T':
            return 40;
        case 'Z':
            return 50;
        case 'R':
            return 90;
        case 'M':
            return 80;
        default:
            return 0;
    }
}

void CheckMovable(int movableList[5][5], int x, int y, char koma){
    switch(koma){
        case 'F'://20
            if(y - 1 >= 0){
                movableList[y - 1][x] += 20;
            }
            break;
        case 'O':
        case 'G':
            for(int k = -1; k <= 1; k++){
                for(int l = -1; l <= 1; l++){
                    if(k == 0 && l == 0) continue;
                    if(y + k < 0 || y + k > 4 || x + l < 0 || x + l > 4) continue;
                        movableList[y + k][x + l] += 0;
                }
            }
            break;
        case 'S'://11
            if(y > 0){
                for(int l = -1; l <= 1; l++){
                    if(x + l < 0 || x + l > 4) continue;
                    movableList[y - 1][x + l] += 11;
                }
            }
            if(y < 4){
                for(int l = -1; l <= 1; l++){
                    if(l == 0) continue;
                    if(x + l < 0 || x + l > 4) continue;
                    movableList[y + 1][x + l] += 11;
                }
            }
            break;
        case 'K'://10
        case 'T'://10
        case 'Z'://10
            for(int k = -1; k <= 0; k++){
                for(int l = -1; l <= 1; l++){
                    if(k == 0 && l == 0) continue;
                    if(y + k < 0 || y + k > 4 || x + l < 0 || x + l > 4) continue;
                    movableList[y + k][x + l] += 10;
                }
            }
            if(y < 4){
                movableList[y + 1][x] += 10;
            }
            break;
        case 'R'://5
            for(int k = -1; k <= 1; k++){
                for(int l = -1; l <= 1; l++){
                    if(k == 0 || l == 0) continue;
                    if(y + k < 0 || y + k > 4 || x + l < 0 || x + l > 4) continue;
                    movableList[y + k][x + l] += 5;
                }
            }
        case 'H'://5
            for(int k = 0; k < 5; k++){
                if(k == x) continue;
                movableList[y][k] += 5;
            }
            for(int k = 0; k < 5; k++){
                if(k == y) continue;
                movableList[k][x] += 5;
            }
            break;
        case 'M'://6
            for(int k = -1; k <= 1; k++){
                for(int l = -1; l <= 1; l++){
                    if(k - l != 1 && k - l != -1) continue;
                    if(y + k < 0 || y + k > 4 || x + l < 0 || x + l > 4) continue;
                    movableList[y + k][x + l] += 6;
                }
            }
        case 'A'://6
            for(int k = 1; k < 5; k++){
                if(y + k > 4 || x + k > 4) break;
                movableList[y + k][x + k] += 6;
            }
            for(int k = 1; k < 5; k++){
                if(y - k < 0 || x + k > 4) break;
                movableList[y - k][x + k] += 6;
            }
            for(int k = 1; k < 5; k++){
                if(y - k < 0 || x - k < 0) break;
                movableList[y - k][x - k] += 6;
            }
            for(int k = 1; k < 5; k++){
                if(y + k > 4 || x - k < 0) break;
                movableList[y + k][x - k] += 6;
            }
            break;
        default :
            ;
    }
}

typedef struct{
    int n_row, n_column;
    double* parameters;
} mat;

void Free(mat a){
    free(a.parameters);
}

double* Board_One_Hottize(char board[5][5][2], int side, int my_side){
    double* board_one_hot=calloc(2000, sizeof(double));
    int turnIndex=side*2+my_side;
    int i;
    for(i=0; i<2000; i++){
        board_one_hot[i]=0;
    }
    int koma_num;
    for(i=0; i<25; i++){
        switch(board[i/5][i%5][0]){
            case 'F':
                koma_num=0;
                break;
            case 'G':
            case 'O':
                koma_num=1;
                break;
            case 'K':
                koma_num=2;
                break;
            case 'S':
                koma_num=3;
                break;
            case 'H':
                koma_num=4;
                break;
            case 'A':
                koma_num=5;
                break;
            case 'T':
                koma_num=6;
                break;
            case 'Z':
                koma_num=7;
                break;
            case 'R':
                koma_num=8;
                break;
            case 'M':
                koma_num=9;
                break;
            default:
                break;
        }
        if(board[i/5][i%5][1]=='U') board_one_hot[(turnIndex*2+0)*250+koma_num*25+i]=1;
        if(board[i/5][i%5][1]=='D') board_one_hot[(turnIndex*2+1)*250+koma_num*25+i]=1;
    }
    return board_one_hot;
}
  
double* Posessed_One_Hottized(char posessed[2][12], int side, int my_side){
    double* posessed_num=calloc(40, sizeof(double));
    int turnIndex=side*2+my_side;
    int i;
    for(i=0; i<40; i++){
        posessed_num[i]=0;
    }
    for(i=0; i>12; i++){
        char koma=posessed[0][i];
        if(koma=='x') break;
        switch(koma){
            case 'F':
                posessed_num[(turnIndex*2+0)*5+0]+=1;
                break;
            case 'K':
                posessed_num[(turnIndex*2+0)*5+1]+=1;
                break;
            case 'S':
                posessed_num[(turnIndex*2+0)*5+2]+=1;
                break;
            case 'A':
                posessed_num[(turnIndex*2+0)*5+3]+=1;
                break;
            case 'H':
                posessed_num[(turnIndex*2+0)*5+4]+=1;
                break;
            default:
                break;
        }
    }
    for(i=0; i>12; i++){
        char koma=posessed[1][i];
        if(koma=='x') break;
        switch(koma){
            case 'F':
                posessed_num[(turnIndex*2+1)*5+0]+=1;
                break;
            case 'K':
                posessed_num[(turnIndex*2+1)*5+1]+=1;
                break;
            case 'S':
                posessed_num[(turnIndex*2+1)*5+2]+=1;
                break;
            case 'A':
                posessed_num[(turnIndex*2+1)*5+3]+=1;
                break;
            case 'H':
                posessed_num[(turnIndex*2+1)*5+4]+=1;
                break;
            default:
                break;
        }
    }
    return posessed_num;
}
  

mat Matricize(int n_row, int n_column, double* parameters){
    mat new_mat;
    new_mat.n_row=n_row;
    new_mat.n_column=n_column;
    new_mat.parameters=calloc(n_row*n_column, sizeof(double));
    int i;
    for(i=0; i<n_row*n_column; i++) new_mat.parameters[i]=parameters[i];
    return new_mat;
}

double MatGet(int row, int column, mat m){
    double ret=m.parameters[row*m.n_column+column];
    return ret;
}

typedef struct{
    mat weight, bias;
} Affine;

mat MatDot(mat a, mat b){
    if(a.n_column!=b.n_row){
        printf("Matrix Size Conflict!\n");
        exit(-1);
    }
    int plus_n=a.n_column;
    int i, j, k;
    double* parameters=calloc(a.n_row*b.n_column, sizeof(double));
    for(j=0; j<a.n_row; j++) for(k=0; k<b.n_column; k++){
        parameters[j*b.n_column+k]=0;
        for(i=0; i<plus_n; i++){
            parameters[j*b.n_column+k]+=MatGet(j, i, a)*MatGet(i, k, b);
        }
    }
    mat m=Matricize(a.n_row, b.n_column, parameters);
    free(parameters);
    Free(a);
    return m;
}

mat MatPlus(mat a, mat b){
    if(a.n_column!=b.n_column || a.n_row!=b.n_row){
        printf("Matrix Size Conflict!\n");
        exit(-1);
    }
    int j, k;
    double* parameters=calloc(a.n_row*a.n_column, sizeof(double));
    for(j=0; j<a.n_row; j++) for(k=0; k<a.n_column; k++){
        parameters[j*b.n_column+k]=MatGet(j, k, a)+MatGet(j, k, b);
    }
    mat m=Matricize(a.n_row, a.n_column, parameters);
    free(parameters);
    Free(a);
    return m;
}

Affine Affinize(mat weight, mat bias){
    Affine aff;
    if(weight.n_column!=bias.n_column){
        printf("Matrix Size Conflict!\n");
        exit(-1);
    }
    //printf("Affine Layer %d -> %d made.\n", weight.n_row, weight.n_column);
    aff.weight=weight;
    aff.bias=bias;
    return aff;
}

mat aff_forward(Affine aff, mat m){
    return MatPlus(MatDot(m, aff.weight), aff.bias);
}

mat ReLU(mat a){
    int i;
    double* parameters=calloc(a.n_row*a.n_column, sizeof(double));
    for(i=0; i<a.n_row*a.n_column; i++){
        parameters[i]=(a.parameters[i]>0 ? a.parameters[i] : 0);
    }
    mat m=Matricize(a.n_row, a.n_column, parameters);
    free(parameters);
    Free(a);
    return m;
}

#define CLAMP_THRESHOLD 0.000001
mat Scaled_Sigmoid(mat a, double scale){
    int i;
    double* parameters=calloc(a.n_row*a.n_column, sizeof(double));
    for(i=0; i<a.n_row*a.n_column; i++){
        double clamped=(a.parameters[i]>CLAMP_THRESHOLD ? a.parameters[i] : CLAMP_THRESHOLD);
        parameters[i]=scale/(1+exp(-clamped));
    }
    mat m=Matricize(a.n_row, a.n_column, parameters);
    free(parameters);
    Free(a);
    return m;
}
#undef CLAMP_THRESHOLD

mat Concatinate(mat a, mat b){
    int i, j;
    if(a.n_row!=b.n_row){
        printf("Matrix Size Conflict!\n");
        exit(-1);
    }
    double* parameters=calloc(a.n_row*(a.n_column+b.n_column), sizeof(double));
    for(i=0; i<a.n_row; i++){
        for(j=0; j<a.n_column; j++){
            parameters[i*a.n_row+j]=MatGet(i, j, a);
        }
        for(j=0; j<b.n_column; j++){
            parameters[i*a.n_row+a.n_column+j]=MatGet(i, j, b);
        }
    }
    mat m=Matricize(a.n_row, a.n_column+b.n_column, parameters);
    free(parameters);
    Free(a);
    Free(b);
    return m;
}



#define MATRICIZE(X) Matricize(X##_n_rows, X##_n_columns, X)
double Forward(char check_board[5][5][2], char check_possessed[2][12], int side, int my_side){
    mat FC1_1W=MATRICIZE(fc1_1_weight);
    mat FC1_1B=MATRICIZE(fc1_1_bias);
    mat FC2_1W=MATRICIZE(fc2_1_weight);
    mat FC2_1B=MATRICIZE(fc2_1_bias);
    mat FC1_2W=MATRICIZE(fc1_2_weight);
    mat FC1_2B=MATRICIZE(fc1_2_bias);
    mat FC2_2W=MATRICIZE(fc2_2_weight);
    mat FC2_2B=MATRICIZE(fc2_2_bias);
    mat FC3W=MATRICIZE(fc3_weight);
    mat FC3B=MATRICIZE(fc3_bias);
    mat FC4W=MATRICIZE(fc4_weight);
    mat FC4B=MATRICIZE(fc4_bias);
    mat FC5W=MATRICIZE(fc5_weight);
    mat FC5B=MATRICIZE(fc5_bias);
    
    Affine FC1_1=Affinize(FC1_1W, FC1_1B);
    Affine FC1_2=Affinize(FC1_2W, FC1_2B);
    Affine FC2_1=Affinize(FC2_1W, FC2_1B);
    Affine FC2_2=Affinize(FC2_2W, FC2_2B);
    Affine FC3=Affinize(FC3W, FC3B);
    Affine FC4=Affinize(FC4W, FC4B);
    Affine FC5=Affinize(FC5W, FC5B);
    
    double* a=Board_One_Hottize(check_board,side, my_side);
    double* b=Posessed_One_Hottized(check_possessed, side, my_side);
    int a_n_rows=1, b_n_rows=1, a_n_columns=2000, b_n_columns=40;
    mat A=MATRICIZE(a);
    mat B=MATRICIZE(b);
    A=aff_forward(FC1_1, A);
    A=ReLU(A);
    A=aff_forward(FC2_1, A);
    B=aff_forward(FC1_2, B);
    B=ReLU(B);
    B=aff_forward(FC2_2, B);
    mat C=Concatinate(A, B);
    C=ReLU(C);
    C=aff_forward(FC3, C);
    C=ReLU(C);
    C=aff_forward(FC4, C);
    C=ReLU(C);
    C=aff_forward(FC5, C);
    C=Scaled_Sigmoid(C, 500);
    double ret=C.parameters[0];
    Free(FC1_1W);
    Free(FC1_1B);
    Free(FC1_2W);
    Free(FC1_2B);
    Free(FC2_1W);
    Free(FC2_1B);
    Free(FC2_2W);
    Free(FC2_2B);
    Free(FC3W);
    Free(FC3B);
    Free(FC4W);
    Free(FC4B);
    Free(FC5W);
    Free(FC5B);
    Free(C);
    return ret;
}
#undef MATRICIZE

#define ML_FLAG 2
//盤面の状態(board,possessed,side)をもらって整数を返す評価関数
int evaluate_Fanction(char check_board[5][5][2], char check_possessed[2][12], int side, int my_side){
    
    
    int value = 0;
    int senteMovable[5][5] = {0};
    int goteMovable[5][5] = {0};
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 12; j++){
            if((check_possessed[i][j] == 'O' && my_side == 1) || (check_possessed[i][j] == 'G' && my_side == 0)) return 1000000;
            if((check_possessed[i][j] == 'G' && my_side == 1) || (check_possessed[i][j] == 'O' && my_side == 0)) return -1000000;
            if(my_side == i){
                value += Omomi(check_possessed[i][j]);
            }else{
                value -= Omomi(check_possessed[i][j]);
            }
        }
    }
    

    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            if((my_side == 0 && check_board[i][j][1] == 'U') || (my_side == 1 && check_board[i][j][1] == 'D')){
                value += Omomi(check_board[i][j][0]);
            }else{
                value -= Omomi(check_board[i][j][0]);
            }

            if(check_board[i][j][1] == 'U'){
                CheckMovable(senteMovable, i, j, check_board[i][j][0]);
            }else if(check_board[i][j][1] == 'D'){
                CheckMovable(goteMovable, i, j, check_board[4 - i][4 - j][0]);
            }
        }
    }

    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            int a; //駒の種類による重要度
            switch(check_board[i][j][0]){
                case 'O':
                case 'G':
                    a = 7; break;
                case 'R':
                    a = 6; break;
                case 'M':
                case 'H':
                    a = 5; break;
                case 'A':
                    a = 4; break;
                case 'K':
                    a = 3; break;
                case 'S':
                case 'T':
                case 'Z':
                    a = 2; break;
                case 'F':
                default:
                    a = 1; break;
            }
            if(my_side == 0){
                if(side == 0){
                    if(check_board[i][j][1] == 'D'){
                        value += (senteMovable[i][j] - goteMovable[i][j]) * a;
                    }
                }else{
                    if(check_board[i][j][1] == 'U'){
                        value += (senteMovable[i][j] - goteMovable[i][j]) * a;
                    }
                }
            }else{
                if(side == 0){
                    if(check_board[i][j][1] == 'D'){
                        value += (senteMovable[i][j] - goteMovable[i][j]) * a;
                    }
                }else{
                    if(check_board[i][j][1] == 'U'){
                        value += (senteMovable[i][j] - goteMovable[i][j]) * a;
                    }
                }
            }
        }
    }

    int score=0;
    char now_board[5][5][2];
    char now_possessed[2][12];
    //一旦グローバル変数を変更
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            for(int k = 0; k < 2; k++){
                now_board[i][j][k] = board[i][j][k];
                board[i][j][k] = check_board[i][j][k];
            }
        }
    }
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 12; j++){
            now_possessed[i][j] = possessed[i][j];
            possessed[i][j] = check_possessed[i][j];
        }
    }
    /*if(side!=my_side){
        if(isOte(side)==-1){score=-1000000;}
        else if(isOte(my_side)==-1){score=10000000;}
        else if(Process_Sen_Nichi_Te(0,side)>=4 && my_side==0){score=-1000000;}
        else if(Process_Sen_Nichi_Te(0,side)>=4 && isOte(my_side)){score=-1000000;}
        else if(isTsumi(side)){score=100000;}
        else if(isOte(side)){score=-100000;}
    }*/
    if(side!=my_side){
        if(isOte(my_side)){score=1000000;}
        if(isTsumi(my_side)){score=-1000000;}
    }else{
        if(isTsumi(side)){score=1000000;}
        if(isOte(1-side)){score=-1000000;}
    }

    //グローバル変数を元に戻す
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            for(int k = 0; k < 2; k++){
                board[i][j][k] = now_board[i][j][k];
            }
        }
    }
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 12; j++){
            possessed[i][j] = now_possessed[i][j];
        }
    }
    if(score!=0) return score;

    //確実な勝ち負けを除いてMLによる推測値を使う場合
    if(my_side==ML_FLAG){
        double val_double=Forward(check_board, check_possessed, side, my_side);
        return (int)val_double+value;}
    
    return value;
}
#undef ML_FLAG

//アルファベータ法をする関数
int Alpha_Beta_algorithm(char board[5][5][2], char possessed[2][12], int side, int my_side, int depth, int alpha, int beta){
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 12; j++){
            if(possessed[i][j] == 'O' || possessed[i][j] == 'G'){ //王か玉が取られてたら探索終わり
                depth = 0;
                break;
            }else if(possessed[i][j] == 'x'){
                break;
            }
        }
    }
    if(depth == 0){
        return evaluate_Fanction(board, possessed, side, my_side);
    }
    char inst_list[500][6];
    int inst_count = search_Next(board, possessed, side, inst_list);
    char next_board[5][5][2];
    char next_possessed[2][12];
    if(side == my_side){
        for(int n = 0; n < inst_count; n++){
            for(int i = 0; i < 5; i++){
                for(int j = 0; j < 5; j++){
                    for(int k = 0; k < 2; k++){
                        next_board[i][j][k] = board[i][j][k];
                    }
                }
            }
            for(int i = 0; i < 2; i++){
                for(int j = 0; j < 12; j++){
                    next_possessed[i][j] = possessed[i][j];
                }
            }
            Move(next_board, next_possessed, side, inst_list[n]);
            int score = Alpha_Beta_algorithm(next_board, next_possessed, 1-side, my_side, depth-1, alpha, beta);
            if(alpha < score){
                alpha = score;
            }
            if(alpha >= beta){
                break;
            }
        }
        return alpha;
    }
    else{
        for(int n = 0; n < inst_count; n++){
            for(int i = 0; i < 5; i++){
                for(int j = 0; j < 5; j++){
                    for(int k = 0; k < 2; k++){
                        next_board[i][j][k] = board[i][j][k];
                    }
                }
            }
            for(int i = 0; i < 2; i++){
                for(int j = 0; j < 12; j++){
                    next_possessed[i][j] = possessed[i][j];
                }
            }
            Move(next_board, next_possessed, side, inst_list[n]);
            int score = Alpha_Beta_algorithm(next_board, next_possessed, 1-side, my_side, depth-1, alpha, beta);
            if(beta > score){
                beta = score;
            }
        }
        return beta;
    }
}

//アルファベータ法で見つけた最善手を返す関数
//正確にはこの関数内ではアルファベータ法はしていない
char* Alpha_Beta(int side){
    char inst_list[500][6];
    int inst_count = search_Next(board, possessed, side, inst_list);
    int max = -999999999;
    int inst_index = 0;
    int depth = 1;
    char *return_inst;
    char next_board[5][5][2];
    char next_possessed[2][12];
    for(int n = 0; n < inst_count; n++){
        //printf("命令:%s\n", inst_list[inst_index]);
        for(int i = 0; i < 5; i++){
            for(int j = 0; j < 5; j++){
                for(int k = 0; k < 2; k++){
                    next_board[i][j][k] = board[i][j][k];
                }
            }
        }
        for(int i = 0; i < 2; i++){
            for(int j = 0; j < 12; j++){
                next_possessed[i][j] = possessed[i][j];
            }
        }
        Move(next_board, next_possessed, side, inst_list[n]);
        int score = Alpha_Beta_algorithm(next_board, next_possessed, 1-side, side, depth, -999999999, 999999999);
        if(max < score){
            max = score;
            inst_index = n;
        }
    }
    return_inst = inst_list[inst_index];
    return return_inst;
}


//千日手の処理のためにAVL木を使っています。
int algebric_max_of_2_value(int a, int b){ return (a>b ? a : b);}

//2つの盤面が等しい場合は0、1つ目のほうが適当な全順序で大きい場合は1、小さい場合は-1を返す関数。
int AVL_TREE_is_Larger_Index_Board(char key_to_be_found[6][5][2],char key_here[6][5][2]){
    int i=0, j=0;
    for(i=0; i<6; i++){
        for(j=0; j<5; j++){
            if(key_to_be_found[i][j][0]>key_here[i][j][0]) return 1;
            if(key_to_be_found[i][j][0]<key_here[i][j][0]) return -1;
            if(key_to_be_found[i][j][1]>key_here[i][j][1]) return 1;
            if(key_to_be_found[i][j][1]<key_here[i][j][1]) return -1;
        }
    }
    return 0;
}

void AVL_TREE_init_node(AVL_TREE_Node* n){
    n->left=NULL;
    n->right=NULL;
    n->height=1;
    n->balance=0;
}

void AVL_TREE_bal_cal(AVL_TREE_Node* n){
    if(n->right==NULL){
        if(n->left==NULL){
            n->balance=0;
            n->height=1;
        }
        else{
            n->balance=n->left->height;
            n->height=1+n->left->height;
        }
    }
    else{
        if(n->left==NULL){
            n->balance=-(n->right->height);
            n->height=1+n->right->height;
        }
        else{
            n->balance=(n->left->height)-(n->right->height);
            n->height=1+algebric_max_of_2_value(n->left->height, n->right->height);
        }
    }
}

void AVL_TREE_set_data(AVL_TREE_Node* n, char key[6][5][2], int value){
    int i, j;

    //for debug(AVL木に放り込む状態の出力)
    //for(i=0; i<6; i++){ for(j=0; j<5; j++){
    //    printf("%c%c ", key[i][j][0], key[i][j][1]);
    //}printf("\n");}
    
    for(i=0; i<6; i++){
        for(j=0; j<5; j++){
            n->key[i][j][0]=key[i][j][0];
            n->key[i][j][1]=key[i][j][1];
        }
    }
    n->value=value;
}

int AVL_TREE_search(AVL_TREE_Node* n, char key[][5][2]){
    int i, j;
    
    if(n==NULL) return -1;
    
    
    int comp_value=AVL_TREE_is_Larger_Index_Board(key,n->key);
    if(comp_value==1){
        return AVL_TREE_search(n->right, key);
    }
    else if(comp_value==-1){
        return AVL_TREE_search(n->left, key);
    }
    else{
        return n->value;
    }
}

AVL_TREE_Node* AVL_TREE_insert(AVL_TREE_Node* n, char key[6][5][2], int value, int balancing){
    if(n==NULL){
        AVL_TREE_Node* newnode = calloc(1, sizeof(AVL_TREE_Node));
        AVL_TREE_init_node(newnode);
        AVL_TREE_set_data(newnode, key, value);
        return newnode;
    }
    int comp_value=AVL_TREE_is_Larger_Index_Board(key,n->key);
    if(comp_value==0){
        AVL_TREE_set_data(n, key, value);
        return n;
    }
    if(comp_value==1) n->right=AVL_TREE_insert(n->right, key, value, balancing);
    else n->left=AVL_TREE_insert(n->left, key, value, balancing);
    if(balancing==0) return n;
    AVL_TREE_bal_cal(n);
    return AVL_TREE_balancize(n);
}

AVL_TREE_Node* AVL_TREE_balancize(AVL_TREE_Node* n){
    //printf("%d\n", n->balance);
    //Print4Debug(n, 0);

    if((n->balance)>1){
        if(n->left->balance>=0) return AVL_TREE_rotate(n, 1);
        else{
            n->left=AVL_TREE_rotate(n->left, 0);
            return AVL_TREE_rotate(n, 1);
        }
    }
    else{
        if((n->balance)<-1){
            if(n->right->balance<=0) return AVL_TREE_rotate(n, 0);
            else{
                n->right=AVL_TREE_rotate(n->right, 1);
                return AVL_TREE_rotate(n, 0);
            }
        }
        else return n;
    }
}

AVL_TREE_Node* AVL_TREE_rotate(AVL_TREE_Node* n, int isRight){
    AVL_TREE_Node* newn;
    if(isRight==0){
        newn=n->right;
        n->right=newn->left;
        newn->left=n;
    }
    else{
        newn=n->left;
        n->left=newn->right;
        newn->right=n;
    }
    AVL_TREE_bal_cal(n);
    AVL_TREE_bal_cal(newn);
    return newn;
}


//以下は使っていないコードの保管場所です(無視してください。)
//下のような形の三次元行列で渡す。
//{*, *, 0}(手前側の駒)、{*, *, 1}(奥側の駒)、{*, *, 2}(手前側の駒で成り)、{*, *, 3}(奥側の駒で成り)
//持ち駒の場合は{-1, -1, 0}(手前側の持ち駒)、{-1, -1, 1}(奥側の持ち駒)
//int game_status[6][2][3]={{{1, 1, 0}, {5, 5, 1}}, //王または玉
//{{1, 2, 0}, {5, 4, 1}}, //金
//    {{1, 3, 0}, {5, 3, 1}}, //銀
//    {{1, 4, 0}, {5, 2, 1}}, //角
//    {{1, 5, 0}, {5, 1, 1}}, //飛
//    {{2, 1, 0}, {4, 5, 1}}}; //歩


//配列boardやpossessedのコマの名前の表記を配列game_statusの何番目の要素に対応しているかを返す
int Index_Of_Array_game_status(char koma_name){
    switch(koma_name){
        case 'G': return 0;
        case 'O': return 0;
        case 'K': return 1;
        case 'S': return 2;
        case 'A': return 3;
        case 'H': return 4;
        case 'F': return 5;
        case 'T': return 5;
        case 'Z': return 2;
        case 'R': return 4;
        case 'M': return 3;
        default: return -1;
    }
}
