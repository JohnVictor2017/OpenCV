// Bibliotecas Padrao
#include <iostream>
#include <vector>
#include <string>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/ml.hpp>

// TinyDir
#include "tinydir.h"

// Testes do tempo de execucao
#include <ctime>

using namespace std;
using namespace cv;

// Rotulos dos arquivos de treino
vector<int> labels;
vector<string> trainingFilenames;


//Pega os arquivos e teste e salva os rotulos/arquvios em uma classe vector
void getsTrainingFiles(){

    //Abre o diretorio que contem as imagens 
    tinydir_dir training_root_dir;
    tinydir_open(&training_root_dir, "training_files");

    // itera pelo diretorio 
    while (training_root_dir.has_next){
        // pega o arquivo
        tinydir_file file;
        tinydir_readfile(&training_root_dir, &file);

        // se for um diretorio 
        if (file.is_dir){

            string numbersDirName = file.name;

            // pula . / .. / .DS_Store (OSX)
            if (numbersDirName != "." && numbersDirName != ".." && numbersDirName != ".DS_Store"){

                // atoi nao é muito seguro, porem suficiente para testes 
                int currentLabel = atoi(file.name);

                // pega o diretorio com os aquivos de numeros
                numbersDirName.insert(0, "training_files/");

                // abre o diretorio que contem os numeros
                tinydir_dir training_number_dir;
                tinydir_open(&training_number_dir, numbersDirName.c_str());

                // itera pelo diretorio
                while (training_number_dir.has_next) {
                    //pega o aquivo
                    tinydir_file trainingJpgFile;
                    tinydir_readfile(&training_number_dir, &trainingJpgFile);

                    // pega o nome
                    string trainingJpgFileName = trainingJpgFile.name;

                    // pula . / .. / .DS_Store (OSX)
                    if (trainingJpgFileName != "." && trainingJpgFileName != ".." && trainingJpgFileName != ".DS_Store"){

                        // diretorio foi iterado completamente
                        trainingJpgFileName.insert(0, numbersDirName + "/");

                        //salva  o nome do arquivo de treinamento e seu rotulo
                        trainingFilenames.push_back(trainingJpgFileName);
                        labels.push_back(currentLabel);
                    }

                    // pega o proximo arquivo
                    tinydir_next(&training_number_dir);
                }

                // fecha o diretorio
                tinydir_close(&training_number_dir);
            }
        }

        //pega o proximo arquivo
        tinydir_next(&training_root_dir);
    }

    // fecha o diretorio
    tinydir_close(&training_root_dir);
}


int main(int argc, char **argv){
    // pega os arquivos de treino
    getsTrainingFiles();

    // dimensao das imagens 
    int imgArea = 28 * 28;

    //armazena os dados de treinamento
    Mat trainingMat(trainingFilenames.size(), imgArea, CV_32FC1);

    int i = clock();
    //itera pelos arquivos de treinamento
    cout << endl << "Analizando rotulos ......." << endl;
    for (int index = 0; index < trainingFilenames.size(); index++){
    
        //Mostra em qual arquivo estamos treinando
        //cout << "Analizando rotulo -> arquivo: " << labels[index] << "|" << trainingFilenames[index] << endl;

        // le a imagem(grayscale)
        Mat imgMat = imread(trainingFilenames[index], 0);

        int ii = 0; //coluna atual em training_mat

        //Processa os pixels individualmente para formar o array 1D da imagem
        for (int i = 0; i < imgMat.rows; i++){
            for (int j = 0; j < imgMat.cols; j++){
                trainingMat.at<float>(index, ii++) = imgMat.at<uchar>(i, j);
            }
        }
    }
    int f = clock();
    cout << "A analise dos rotulos levou: " << (f-i)/(float)CLOCKS_PER_SEC << "s" << endl;

    //Processa os rotulos 

    int labelsArray[labels.size()];

    // itera pelos rotulos
    for (int index = 0; index < labels.size(); index++){
        labelsArray[index] = labels[index];
    }

    Mat labelsMat(labels.size(), 1, CV_32S, labelsArray);

    // Configura a SVM
    // 'Seta' os parametros(optimal(ish)) da SVM's
    Ptr<ml::SVM> svm = ml::SVM::create();
    svm->setType(ml::SVM::C_SVC);
    svm->setKernel(ml::SVM::POLY);
    svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));
    svm->setGamma(3);
    svm->setDegree(3);

    //Treina o classificador 
    i = clock();
    cout << "Treinando o classificador......." << endl;
    svm->train(trainingMat, ml::ROW_SAMPLE, labelsMat);
    f = clock();
    cout << "O treinamento levou " << (f-i)/(float)CLOCKS_PER_SEC << "s" << endl;

    // Salva a SVM
    cout << "Salvando a SVM......." << endl;
    svm->save("classifier.yml");
    cout << endl;
}