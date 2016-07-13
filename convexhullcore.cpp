#include "convexhullcore.h"

ConvexHullCore::ConvexHullCore(DrawableDcel *dcel)
{
    this->dcel    = dcel;
    this->vertexS = std::vector<Dcel::Vertex*>(dcel->getNumberVertices());

}

/**
 * @brief ConvexHullCore::VerifyEuleroProperty()
 * This method is executed to verify the Eurler's formula (n-ne+nf=2)
 * @return True if the dcel respects the property, False otherwhise
 */
bool ConvexHullCore::verifyEuleroProperty(){
    /*
    cout << "Numero di facce " <<dcel->getNumberFaces()<< endl;
    cout << "Numero di vertici " <<dcel->getNumberVertices()<< endl;
    cout << "Numero di edge " <<dcel->getNumberHalfEdges()/2<<endl;
    */
    int numberFace  = dcel->getNumberFaces();
    int numberVertex= dcel->getNumberVertices();
    int numberEdge  = dcel->getNumberHalfEdges()/2;

    int euler=numberVertex-numberFace+numberEdge;

    if(euler==2){
        return true;
    }else{
        return false;
    }
}

/**
 * @brief ConvexHullCore::getVertexs()
 * This method is executed to get all the vertex of the dcel
 */
void ConvexHullCore::getVertexs(){
    int n=0;
    Dcel::VertexIterator vit;    
    for(vit = this->dcel->vertexBegin(); vit != this->dcel->vertexEnd(); ++vit){
        this->vertexS[n] = *vit;
        n++;
    }
}

/**
 * @brief ConvexHull::isCoplanar()
 * This method is execut to verify if the 4 points are coplanar
 * @return True if all the 4 points are coplanar, false otherwise
 * http://mathworld.wolfram.com/Coplanar.html
 * In this method is used eigen library, it can easily perform the
 * determinant of a matrix
 */
bool ConvexHullCore::isCoplanar(){

    Pointd p0= vertexS[0]->getCoordinate();
    Pointd p1= vertexS[1]->getCoordinate();
    Pointd p2= vertexS[2]->getCoordinate();
    Pointd p3= vertexS[3]->getCoordinate();

    Matrix<double,4,4> matrix;
    matrix<< p0.x(), p0.y(), p0.z(), 1,
             p1.x(), p1.y(), p1.z(), 1,
             p2.x(), p2.y(), p2.z(), 1,
             p3.x(), p3.y(), p3.z(), 1;

    double det = matrix.determinant();

    if(det > std::numeric_limits<double>::epsilon() || det < -std::numeric_limits<double>::epsilon()){
        return false; //zucchero sintattico, fa un return 0
    }else{
        return true; // fa un return 1
    }
}

/**
 * @brief ConvexHullCore::getPermutation()
 * This method is executed to execute the permutation of the vertexs
 * http://www.cplusplus.com/reference/algorithm/random_shuffle/
 */
void ConvexHullCore::executePermutation(){
    do{
       std::random_shuffle(this->vertexS.begin(), this->vertexS.end());
    }while(isCoplanar());
}


/**
 * @brief ConvexHullCore::setTetrahedron
 * This method is used to set the first 4 points to create the initial tetrahedron. In the dcel, after
 * the execution of setTetrahedron() the dcel contain the convex hull of the 4 initial points
 */
void ConvexHullCore::setTetrahedron(){
    //CREAZIONE TETRAEDRO

    //CREAZIONE TRIANGOLO
    std::vector<Dcel::Vertex*> vertexInitial = std::vector<Dcel::Vertex*>(3); //conterrà i vertici per formare la base
    std::vector<Dcel::HalfEdge*> heInTriangle = std::vector<Dcel::HalfEdge*>(3);//conterrà gli halfedge per formare la base

    //HalfEdge del Tetraedro
    std::vector<Dcel::HalfEdge*> heInTetrahedron = std::vector<Dcel::HalfEdge*>(9);

    //Creazione vertici della Base
    for(unsigned int i=0;i<3;i++){
        vertexInitial[i] =this->dcel->addVertex(*this->vertexS[i]);
        heInTriangle[i]= this->dcel->addHalfEdge();
    }

    //Creazione faccia della base
    Dcel::Face* f1= this->dcel->addFace();

    //Ciclo per impostare i next,prev,from,to e face della Base
    for(unsigned int i=0;i<3;i++){//uso il modulo per completare il ciclo
        heInTriangle[i]->setFromVertex(vertexInitial[ i ]);
        heInTriangle[i]->setToVertex(vertexInitial[ (i+1)%3 ]);
        heInTriangle[i]->setNext(heInTriangle[ (i+1)%3 ]);
        heInTriangle[i]->setPrev(heInTriangle[ (i+2)%3 ]);
        heInTriangle[i]->setFace(f1);
        vertexInitial[i]->setIncidentHalfEdge(heInTriangle[ (i+2)%3 ]);
    }
    f1->setOuterHalfEdge(heInTriangle[0]);

    //Aggiunta quarto vertice
    Dcel::Vertex* v4 =this->dcel->addVertex(*this->vertexS[3]);

    //AggiuntaHalfEdgeTetraedro
    for(unsigned int i=0;i<9;i++){
        heInTetrahedron[i]= this->dcel->addHalfEdge();
    }

    //Settaggio half edge tetraedro
    int k=0;//usata per gli half edge del tetraedro
    for(unsigned int i=0;i<3;i++){//ad ogni ciclo creo una faccia F ed imposto i 3 half edge della faccia corrente (F)
        Dcel::Face* face= this->dcel->addFace();

        heInTetrahedron[k]->setFromVertex(vertexInitial[ i ]);
        heInTetrahedron[k]->setToVertex(v4);
        heInTetrahedron[k]->setNext(heInTetrahedron[ (k+1) ]);
        heInTetrahedron[k]->setPrev(heInTetrahedron[ (k+2) ]);
        heInTetrahedron[k]->setFace(face);
        heInTetrahedron[k]->setTwin(heInTetrahedron[ (k+8)%9 ]);

        face->setOuterHalfEdge(heInTetrahedron[k]);

        k++;
        heInTetrahedron[k]->setFromVertex(v4);
        heInTetrahedron[k]->setToVertex(vertexInitial[ (i+1)%3 ]);
        heInTetrahedron[k]->setNext(heInTetrahedron[ (k+1) ]);
        heInTetrahedron[k]->setPrev(heInTetrahedron[ (k-1) ]);
        heInTetrahedron[k]->setFace(face);
        heInTetrahedron[k]->setTwin(heInTetrahedron[ (k+2)%8 ]);

        k++;
        heInTetrahedron[k]->setFromVertex(vertexInitial[ (i+1)%3 ]);
        heInTetrahedron[k]->setToVertex(vertexInitial[ i ]);
        heInTetrahedron[k]->setNext(heInTetrahedron[ (k-2) ]);
        heInTetrahedron[k]->setPrev(heInTetrahedron[ (k-1) ]);
        heInTetrahedron[k]->setFace(face);
        heInTetrahedron[k]->setTwin(heInTriangle[i]);
        heInTriangle[i]->setTwin(heInTetrahedron[k]);

        k++;
    }
}

/**
 * @brief ConvexHullCore::findConvexHull()
 * This method is executed to find the convex hull given a set of points (contains into dcel)
 * This method, is the principal method of the class
 */
void ConvexHullCore::findConvexHull(){
    getVertexs();

    //Calcola una permutazione random degli n punti
    executePermutation();
    
    this->dcel->reset();

    //Trova 4 punti che formano il tetraedro (quindi il convex hull di questi 4 punti)
    setTetrahedron();

    //Inizializza il conflict graph con tutte le coppie visibili (Pt,f) con f faccia in dcel e t>4 (quindi con i punti successivi)
    ConflictGraph conflictGraph=ConflictGraph(this->dcel, this-> vertexS);
    conflictGraph.initializeCG();

        
}