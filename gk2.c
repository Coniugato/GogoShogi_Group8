//2022.12.8 Ryusei Ishikawa Updated.
#include <stdio.h>
#include <stdlib.h>
#define ALPHA_BETA_FUNC_NAME Alpha_Beta

//READ HERE:
//今のところ反則手(関数is_Banned)と千日手の処理(global変数Num_Sen_nichi_teを更新する処理)が未実装です。
//下記のglobal変数群を使いながら、次の関数に渡して最適手の計算を行うことを考えています。
//引数sideは0の時先手(盤の手前側)の手番、0の時後手(盤の奥側)の手番
//返り値はユーザの入力と同様、"2A3B"などの文字列
//引数と返り値に関しては変更するにはコードの手直しが要るので容易ではないですが、関数名については4行目のALPHA_BETA_FUNC_NAMEのマクロの定義を変えてもらえるだけで違う関数名でも動きますので、変更したい場合はどうぞ。
char* Alpha_Beta(int side){
    return "3C3C";
}
//global変数群
//0のときUser、1のときAIが先手となる。2のときいずれもUser、3のときいずれもAI。2及び3はdebug用。
int First_Player=0;
//0の時は通常、1の時は盤面を表示する。
int Display_Board_Enabled=1;
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
//これまでの手数(先手後手合わせて)
int Num_Moves=0;
//千日手の繰り返し回数(未実装)
int Num_Sen_nichi_te=0;





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
    printf("\n\n盤面:\n\n持ち駒(後手):");
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
            printf("\n\n");
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

//反則手かどうかを判定する関数(未実装)
int is_Banned(char* input){
    return 0;
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
            if(sx-gx!=sy-gy){
                int X=(sx>gx ? gx : sx)+1, Y=(sy>gy ? gy : sy)+1, MaximX=(sx<gx ? gx : sx)+1;
                for(; X<MaximX; X++, Y++) if(board[Y][X][0]!='x') return 0;
            }
            else{
                int X=(sx>gx ? gx : sx)+1, Y=(sy<gy ? gy : sy)-1, MaximX=(sx<gx ? gx : sx)+1;
                for(; X<MaximX; X++, Y--) if(board[Y][X][0]!='x') return 0;
            }
            break;
            
        default:
            printf("\nCode error: invalid code: %c\n", koma_name);//for debug
            return 0;
    }
    return 1;
}

//命令列がゲームとして許容されるかを判定する関数(反則手の部分が未完成なので若干不十分な挙動)
int Check_If_Permitted_Move(char* input, int side){
    if(is_Put_Instruction(input)){
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
            if(is_Banned(input)) return 0;
            else return 1;
        }
        else return 0;
    }
    else{
        if((side==0 && board[4-input[0]+'1'][input[1]-'A'][1]=='U') || (side==1 && board[4-input[0]+'1'][input[1]-'A'][1]=='D') ){
            if(is_Movable(board[4-input[0]+'1'][input[1]-'A'][0], 4-input[0]+'1', input[1]-'A', 4-input[2]+'1', input[3]-'A', side)){
                if(is_Banned(input)) return 0; //反則手
            }
            else return 0; //駒としてそこに動けない場合
        }
        else return 0; //自分のコマでなかったり駒がなかったりする場合
        if(intended_Nari(input)){
            int koma_name=board[4-input[0]+'1'][input[1]-'A'][0];
            if(Process_Nari(koma_name)=='?') return 0;
            if(side==1 && 4-input[0]+'1'<3 && 4-input[2]+'1'<3) return 0;
            if(side==0 && 4-input[0]+'1'>1 && 4-input[2]+'1'>1) return 0;
        }
        return 1;
    }
}

//ユーザの手番の処理
void Player_Process(int side){
    char input[10];
    scanf("%s", input);
    if(Check_If_Permitted_Move(input, side)){
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
        printf("You Lose\n");
        exit(0);
    }
    return;
}

//AIの手番の処理
void AI_Process(int side){
    char* input=ALPHA_BETA_FUNC_NAME(side);
    if(Check_If_Permitted_Move(input, side)){
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
int main(void){

    while(1){
        //先手の手番
        if(Display_Board_Enabled) Display_Board(0);
        if(First_Player==0 || First_Player==2){
            Player_Process(0);
        }
        else{
            AI_Process(0);
        }
        Num_Moves++;
        //後手の手番
        if(Display_Board_Enabled) Display_Board(1);
        if(First_Player==1 || First_Player==2){
            Player_Process(1);
        }
        else{
            AI_Process(1);
        }
        Num_Moves++;
        //break; //for debug
    }
    return 0;
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
