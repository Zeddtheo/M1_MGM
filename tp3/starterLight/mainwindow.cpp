#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QRandomGenerator>
#include <queue>
/* **** début de la partie à compléter **** */
void MainWindow::showEdgeSelection(MyMesh* _mesh)
{
    // on réinitialise les couleurs de tout le maillage
    resetAllColorsAndThickness(_mesh);

    /* **** à compléter ! (Partie 1) ****
     * cette fonction utilise la variables de sélection edgeSelection qui est l'ID de
     * l'arête sélectionnée et qui est égale à -1 si la sélection est vide
     */

    if(edgeSelection >= 0 && static_cast<size_t>(edgeSelection) < _mesh->n_edges()){
        EdgeHandle eh = _mesh->edge_handle(edgeSelection);
        _mesh->set_color(eh, MyMesh::Color(0, 255, 0));
        _mesh->data(eh).thickness = 4;

        HalfedgeHandle heh = _mesh->halfedge_handle(eh, 0);

        VertexHandle vh2 = _mesh->from_vertex_handle(heh);
        _mesh->set_color(vh2, MyMesh::Color(0, 255, 0));
        _mesh->data(vh2).thickness = 12;

        VertexHandle vh1 = _mesh->to_vertex_handle(heh);
        _mesh->set_color(vh1, MyMesh::Color(0, 255, 0));
        _mesh->data(vh1).thickness = 12;
    }

    // on affiche le nouveau maillage
    displayMesh(_mesh);
}

void MainWindow::collapseEdge(MyMesh* _mesh, int edgeID)
{
    /* **** à compléter ! (Partie 1) ****
     * cette fonction utilise l'opérateur collapse pour supprimer l'arête d'index edgeID
     * Attention à ne pas oublier garbage_collection() !
     */

    // indispensable pour supprimer des éléments (voir doc https://www.graphics.rwth-aachen.de/media/openmesh_static/Documentations/OpenMesh-6.0-Documentation/a00060.html)
    _mesh->request_vertex_status();
    _mesh->request_edge_status();
    _mesh->request_face_status();

    //first make sure the edge is valid
    if(edgeID >= 0 && static_cast<size_t>(edgeID) < _mesh->n_edges()){
        EdgeHandle eh = _mesh->edge_handle(edgeID);
        //get the halfedge handle
        HalfedgeHandle heh = _mesh->halfedge_handle(eh, 0);
        //check if it is collapsible
        if(_mesh->is_collapse_ok(heh)){
            MyMesh::Point p = _mesh->calc_edge_midpoint(heh);
            //collapse the edge
            _mesh->collapse(heh);
            //use the mid point of the edge as the new vertex position
            _mesh->set_point(_mesh->to_vertex_handle(heh), p);

            // permet de nettoyer le maillage et de garder la cohérence des indices après un collapse
            _mesh->garbage_collection();
        }
    }

}

int MainWindow::randInt(int low, int high){
    //return qrand() % ((high+1)-low)+low;
    return QRandomGenerator::global()->bounded(low, high + 1);
}

void MainWindow::decimation(MyMesh* _mesh, int percent, QString method)
{
    /* **** à compléter ! (Partie 2 et 3) ****
     * Cette fonction supprime des arêtes jusqu'à atteindre un pourcentage d'arêtes restantes, selon un critère donné
     * percent : pourcentage de l'objet à garder
     * method  : la méthode à utiliser parmis : "Aléatoire", "Par taille", "Par angle", "Par planéité"
     */


    if(method == "Aléatoire")
    {
        // get the number of edges
        int nbEdges = _mesh->n_edges();
        //get the number of edges to remove
        int nbEdgesToRemove = _mesh->n_edges() - (_mesh->n_edges() * percent / 100);
        //check if the number of edges to remove is not greater than the number of edges
        if(nbEdgesToRemove > nbEdges){
            nbEdgesToRemove = nbEdges;
        }
        //remove the edges
        for(int i = 0; i < nbEdgesToRemove; i++){
            //get a random edge
            int edgeID = randInt(0, nbEdges - 1);
            //collapse the edge
            collapseEdge(_mesh, edgeID);
        }
        qDebug() << "nbEdgesToRemove : " << nbEdgesToRemove;
    }
    else if(method == "Par taille")
    {
        //we need to compute and compare the length of each edge
        //we need to store the length of each edge
        std::vector<float> edgeLengths;
        //we need to store the edge handles
        std::vector<EdgeHandle> edgeHandles;
        //we need to store the edge IDs
        std::vector<int> edgeIDs;
        //we need to store the edge IDs sorted by length
        std::vector<int> edgeIDsSorted;

        //get the number of edges
        int nbEdges = _mesh->n_edges();
        //get the number of edges to remove
        int nbEdgesToRemove = _mesh->n_edges() - (_mesh->n_edges() * percent / 100);
        //check if the number of edges to remove is not greater than the number of edges
        if(nbEdgesToRemove > nbEdges){
            nbEdgesToRemove = nbEdges;
        }

        //get the edge lengths
        for(int i = 0; i < nbEdges; i++){
            EdgeHandle eh = _mesh->edge_handle(i);
            //get the halfedge handle
            HalfedgeHandle heh = _mesh->halfedge_handle(eh, 0);
            //get the edge length
            float edgeLength = _mesh->calc_edge_length(heh);
            //store the edge length
            edgeLengths.push_back(edgeLength);
            //store the edge handle
            edgeHandles.push_back(eh);
            //store the edge ID
            edgeIDs.push_back(i);
        }

        //sort the edge IDs by length
        edgeIDsSorted = edgeIDs;
        std::sort(edgeIDsSorted.begin(), edgeIDsSorted.end(), [&edgeLengths](int i1, int i2) {return edgeLengths[i1] < edgeLengths[i2];});

        //remove the edges
        for(int i = 0; i < nbEdgesToRemove; i++){
            //get the edge ID
            int edgeID = edgeIDsSorted[i];
            //collapse the edge
            collapseEdge(_mesh, edgeID);
        }
        qDebug() << "nbEdgesToRemove : " << nbEdgesToRemove;

    }
    else if(method == "Par angle")
    {
        //on supprime en priorité les arêtes qui ont des faces voisines qui sont proches d'un plan.
        //on va donc calculer l'angle entre les normales des faces voisines
        //on va stocker les angles
        std::vector<float> edgeAngles;
        //on va stocker les edge handles
        std::vector<EdgeHandle> edgeHandles;
        //on va stocker les edge IDs
        std::vector<int> edgeIDs;
        //on va stocker les edge IDs triés par angle
        std::vector<int> edgeIDsSorted;
        
        //get the number of edges
        int nbEdges = _mesh->n_edges();
        //get the number of edges to remove
        int nbEdgesToRemove = _mesh->n_edges() - (_mesh->n_edges() * percent / 100);
        //check if the number of edges to remove is not greater than the number of edges
        if(nbEdgesToRemove > nbEdges){
            nbEdgesToRemove = nbEdges;
        }

        //get the edge angles
        for(int i = 0; i < nbEdges; i++){
            EdgeHandle eh = _mesh->edge_handle(i);
            //get the halfedge handle
            HalfedgeHandle heh = _mesh->halfedge_handle(eh, 0);
            //get the edge angle
            float edgeAngle = _mesh->calc_dihedral_angle(heh);
            //store the edge angle
            edgeAngles.push_back(edgeAngle);
            //store the edge handle
            edgeHandles.push_back(eh);
            //store the edge ID
            edgeIDs.push_back(i);
        }

        //sort the edge IDs by angle
        edgeIDsSorted = edgeIDs;
        std::sort(edgeIDsSorted.begin(), edgeIDsSorted.end(), [&edgeAngles](int i1, int i2) {return edgeAngles[i1] < edgeAngles[i2];});

        //remove the edges
        for(int i = 0; i < nbEdgesToRemove; i++){
            //get the edge ID
            int edgeID = edgeIDsSorted[i];
            //collapse the edge
            collapseEdge(_mesh, edgeID);
        }
        qDebug() << "nbEdgesToRemove : " << nbEdgesToRemove;

    }
    else if(method == "Par planéité")
    {
        // We will store the edge angles
        std::vector<float> edgeAngles;
        // We will store the edge handles
        std::vector<EdgeHandle> edgeHandles;
        // We will store the edge IDs
        std::vector<int> edgeIDs;
        // We will store the edge IDs sorted by angle
        std::vector<int> edgeIDsSorted;

        // Get the number of edges
        int nbEdges = _mesh->n_edges();
        // Get the number of edges to remove
        int nbEdgesToRemove = _mesh->n_edges() - (_mesh->n_edges() * percent / 100);
        // Check if the number of edges to remove is not greater than the number of edges
        if(nbEdgesToRemove > nbEdges){
            nbEdgesToRemove = nbEdges;
        }

        // Get the edge angles
        for(int i = 0; i < nbEdges; i++){
            EdgeHandle eh = _mesh->edge_handle(i);
            // Get the halfedge handle
            HalfedgeHandle heh = _mesh->halfedge_handle(eh, 0);
            // Get the adjacent faces
            FaceHandle fh0 = _mesh->face_handle(heh);
            FaceHandle fh1 = _mesh->opposite_face_handle(heh);
            // Check if the edge is a boundary edge
            if(fh1.is_valid()){
                // Get the vertices of the halfedge
                VertexHandle vh0 = _mesh->from_vertex_handle(heh);
                VertexHandle vh1 = _mesh->to_vertex_handle(heh);
                // Calculate the angle between the adjacent faces
                float angle = angleFF(_mesh, fh0.idx(), fh1.idx(), vh0.idx(), vh1.idx());
                // Store the edge angle
                edgeAngles.push_back(angle);
            } else {
                // For boundary edges, store a large value so they won't be removed
                edgeAngles.push_back(std::numeric_limits<float>::max());
            }
            // Store the edge handle
            edgeHandles.push_back(eh);
            // Store the edge ID
            edgeIDs.push_back(i);
        }

        // Sort the edge IDs by angle
        edgeIDsSorted = edgeIDs;
        std::sort(edgeIDsSorted.begin(), edgeIDsSorted.end(), [&edgeAngles](int i1, int i2) {return edgeAngles[i1] < edgeAngles[i2];});

        // Remove the edges
        for(int i = 0; i < nbEdgesToRemove; i++){
            // Get the edge ID
            int edgeID = edgeIDsSorted[i];
            // Collapse the edge
            collapseEdge(_mesh, edgeID);
        }
        qDebug() << "nbEdgesToRemove : " << nbEdgesToRemove;
    }

    else
    {
        qDebug() << "Méthode inconnue !!!";
    }

}
float MainWindow::angleFF(MyMesh *_mesh, int faceID0, int faceID1, int vertID0, int vertID1)
{
    /* **** à compléter ! **** */
    FaceHandle fh0 = _mesh->face_handle(faceID0);
    FaceHandle fh1 = _mesh->face_handle(faceID1);
    VertexHandle vh0 = _mesh->vertex_handle(vertID0);
    VertexHandle vh1 = _mesh->vertex_handle(vertID1);

    OpenMesh::Vec3f n0(_mesh->normal(fh0));
    OpenMesh::Vec3f n1(_mesh->normal(fh1));

    MyMesh::Point p0 = _mesh->point(vh0);
    MyMesh::Point p1 = _mesh->point(vh1);

    Vec3f cross_product = OpenMesh::cross(n0, n1);
    Vec3f p = p1 - p0;

    float dot_norm_cross = OpenMesh::dot(p, cross_product);
    int sign = (dot_norm_cross > 0) ? 1 : -1;
    return sign * std::acos(OpenMesh::dot(n0, n1));
}
/* **** début de la partie boutons et IHM **** */
void MainWindow::updateEdgeSelectionIHM()
{
    /* **** à compléter ! (Partie 3) ****
     * Cette fonction met à jour l'interface, les critères pourrons être affichés dans la zone de texte pour les vérifier
     */

    QString infos = "";
    infos = infos + "Surface : " + QString::number(0) + "\n";
    infos = infos + "C1 : " + QString::number(0) + "\n";
    infos = infos + "C2 : " + QString::number(0) + "\n";
    infos = infos + "C3 : " + QString::number(0) + "\n";
    ui->infoEdgeSelection->setPlainText(infos);

    ui->labelEdge->setText(QString::number(edgeSelection));

    // on montre la nouvelle sélection
    showEdgeSelection(&mesh);
}
/* **** fin de la partie à compléter **** */

void MainWindow::on_pushButton_edgeMoins_clicked()
{
    // mise à jour de l'interface
    edgeSelection = edgeSelection - 1;
    updateEdgeSelectionIHM();
}

void MainWindow::on_pushButton_edgePlus_clicked()
{
    // mise à jour de l'interface
    edgeSelection = edgeSelection + 1;
    updateEdgeSelectionIHM();
}

void MainWindow::on_pushButton_delSelEdge_clicked()
{
    // on supprime l'arête d'indice edgeSelection
    collapseEdge(&mesh, edgeSelection);

    // on actualise la sélection
    showEdgeSelection(&mesh);
}

void MainWindow::on_pushButton_chargement_clicked()
{
    // fenêtre de sélection des fichiers
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Mesh"), "", tr("Mesh Files (*.obj)"));

    // chargement du fichier .obj dans la variable globale "mesh"
    OpenMesh::IO::read_mesh(mesh, fileName.toUtf8().constData());

    mesh.update_normals();

    // initialisation des couleurs et épaisseurs (sommets et arêtes) du mesh
    resetAllColorsAndThickness(&mesh);

    // on affiche le maillage
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_decimate_clicked()
{
    decimation(&mesh, ui->horizontalSlider->value(), ui->comboBox->currentText());
    displayMesh(&mesh);
}
/* **** fin de la partie boutons et IHM **** */



/* **** fonctions supplémentaires **** */
// permet d'initialiser les couleurs et les épaisseurs des élements du maillage
void MainWindow::resetAllColorsAndThickness(MyMesh* _mesh)
{
    for (MyMesh::VertexIter curVert = _mesh->vertices_begin(); curVert != _mesh->vertices_end(); curVert++)
    {
        _mesh->data(*curVert).thickness = 1;
        _mesh->set_color(*curVert, MyMesh::Color(0, 0, 0));
    }

    for (MyMesh::FaceIter curFace = _mesh->faces_begin(); curFace != _mesh->faces_end(); curFace++)
    {
        _mesh->set_color(*curFace, MyMesh::Color(150, 150, 150));
    }

    for (MyMesh::EdgeIter curEdge = _mesh->edges_begin(); curEdge != _mesh->edges_end(); curEdge++)
    {
        _mesh->data(*curEdge).thickness = 1;
        _mesh->set_color(*curEdge, MyMesh::Color(0, 0, 0));
    }
}

// charge un objet MyMesh dans l'environnement OpenGL
void MainWindow::displayMesh(MyMesh* _mesh, DisplayMode mode)
{
    GLuint* triIndiceArray = new GLuint[_mesh->n_faces() * 3];
    GLfloat* triCols = new GLfloat[_mesh->n_faces() * 3 * 3];
    GLfloat* triVerts = new GLfloat[_mesh->n_faces() * 3 * 3];

    int i = 0;

    if(mode == DisplayMode::TemperatureMap)
    {
        QVector<float> values;
        for (MyMesh::VertexIter curVert = _mesh->vertices_begin(); curVert != _mesh->vertices_end(); curVert++)
            values.append(fabs(_mesh->data(*curVert).value));
        std::sort(values.begin(), values.end());

        float range = values.at(values.size()*0.8);

        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;

        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            if(_mesh->data(*fvIt).value > 0){triCols[3*i+0] = 255; triCols[3*i+1] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+2] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            else{triCols[3*i+2] = 255; triCols[3*i+1] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+0] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            if(_mesh->data(*fvIt).value > 0){triCols[3*i+0] = 255; triCols[3*i+1] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+2] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            else{triCols[3*i+2] = 255; triCols[3*i+1] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+0] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            if(_mesh->data(*fvIt).value > 0){triCols[3*i+0] = 255; triCols[3*i+1] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+2] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            else{triCols[3*i+2] = 255; triCols[3*i+1] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+0] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }

    if(mode == DisplayMode::Normal)
    {
        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            triCols[3*i+0] = _mesh->color(*fIt)[0]; triCols[3*i+1] = _mesh->color(*fIt)[1]; triCols[3*i+2] = _mesh->color(*fIt)[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->color(*fIt)[0]; triCols[3*i+1] = _mesh->color(*fIt)[1]; triCols[3*i+2] = _mesh->color(*fIt)[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->color(*fIt)[0]; triCols[3*i+1] = _mesh->color(*fIt)[1]; triCols[3*i+2] = _mesh->color(*fIt)[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }

    if(mode == DisplayMode::ColorShading)
    {
        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            triCols[3*i+0] = _mesh->data(*fvIt).faceShadingColor[0]; triCols[3*i+1] = _mesh->data(*fvIt).faceShadingColor[1]; triCols[3*i+2] = _mesh->data(*fvIt).faceShadingColor[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->data(*fvIt).faceShadingColor[0]; triCols[3*i+1] = _mesh->data(*fvIt).faceShadingColor[1]; triCols[3*i+2] = _mesh->data(*fvIt).faceShadingColor[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->data(*fvIt).faceShadingColor[0]; triCols[3*i+1] = _mesh->data(*fvIt).faceShadingColor[1]; triCols[3*i+2] = _mesh->data(*fvIt).faceShadingColor[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }


    ui->displayWidget->loadMesh(triVerts, triCols, _mesh->n_faces() * 3 * 3, triIndiceArray, _mesh->n_faces() * 3);

    delete[] triIndiceArray;
    delete[] triCols;
    delete[] triVerts;

    GLuint* linesIndiceArray = new GLuint[_mesh->n_edges() * 2];
    GLfloat* linesCols = new GLfloat[_mesh->n_edges() * 2 * 3];
    GLfloat* linesVerts = new GLfloat[_mesh->n_edges() * 2 * 3];

    i = 0;
    QHash<float, QList<int> > edgesIDbyThickness;
    for (MyMesh::EdgeIter eit = _mesh->edges_begin(); eit != _mesh->edges_end(); ++eit)
    {
        float t = _mesh->data(*eit).thickness;
        if(t > 0)
        {
            if(!edgesIDbyThickness.contains(t))
                edgesIDbyThickness[t] = QList<int>();
            edgesIDbyThickness[t].append((*eit).idx());
        }
    }
    QHashIterator<float, QList<int> > it(edgesIDbyThickness);
    QList<QPair<float, int> > edgeSizes;
    while (it.hasNext())
    {
        it.next();

        for(int e = 0; e < it.value().size(); e++)
        {
            int eidx = it.value().at(e);

            MyMesh::VertexHandle vh1 = _mesh->to_vertex_handle(_mesh->halfedge_handle(_mesh->edge_handle(eidx), 0));
            linesVerts[3*i+0] = _mesh->point(vh1)[0];
            linesVerts[3*i+1] = _mesh->point(vh1)[1];
            linesVerts[3*i+2] = _mesh->point(vh1)[2];
            linesCols[3*i+0] = _mesh->color(_mesh->edge_handle(eidx))[0];
            linesCols[3*i+1] = _mesh->color(_mesh->edge_handle(eidx))[1];
            linesCols[3*i+2] = _mesh->color(_mesh->edge_handle(eidx))[2];
            linesIndiceArray[i] = i;
            i++;

            MyMesh::VertexHandle vh2 = _mesh->from_vertex_handle(_mesh->halfedge_handle(_mesh->edge_handle(eidx), 0));
            linesVerts[3*i+0] = _mesh->point(vh2)[0];
            linesVerts[3*i+1] = _mesh->point(vh2)[1];
            linesVerts[3*i+2] = _mesh->point(vh2)[2];
            linesCols[3*i+0] = _mesh->color(_mesh->edge_handle(eidx))[0];
            linesCols[3*i+1] = _mesh->color(_mesh->edge_handle(eidx))[1];
            linesCols[3*i+2] = _mesh->color(_mesh->edge_handle(eidx))[2];
            linesIndiceArray[i] = i;
            i++;
        }
        edgeSizes.append(qMakePair(it.key(), it.value().size()));
    }

    ui->displayWidget->loadLines(linesVerts, linesCols, i * 3, linesIndiceArray, i, edgeSizes);

    delete[] linesIndiceArray;
    delete[] linesCols;
    delete[] linesVerts;

    GLuint* pointsIndiceArray = new GLuint[_mesh->n_vertices()];
    GLfloat* pointsCols = new GLfloat[_mesh->n_vertices() * 3];
    GLfloat* pointsVerts = new GLfloat[_mesh->n_vertices() * 3];

    i = 0;
    QHash<float, QList<int> > vertsIDbyThickness;
    for (MyMesh::VertexIter vit = _mesh->vertices_begin(); vit != _mesh->vertices_end(); ++vit)
    {
        float t = _mesh->data(*vit).thickness;
        if(t > 0)
        {
            if(!vertsIDbyThickness.contains(t))
                vertsIDbyThickness[t] = QList<int>();
            vertsIDbyThickness[t].append((*vit).idx());
        }
    }
    QHashIterator<float, QList<int> > vitt(vertsIDbyThickness);
    QList<QPair<float, int> > vertsSizes;

    while (vitt.hasNext())
    {
        vitt.next();

        for(int v = 0; v < vitt.value().size(); v++)
        {
            int vidx = vitt.value().at(v);

            pointsVerts[3*i+0] = _mesh->point(_mesh->vertex_handle(vidx))[0];
            pointsVerts[3*i+1] = _mesh->point(_mesh->vertex_handle(vidx))[1];
            pointsVerts[3*i+2] = _mesh->point(_mesh->vertex_handle(vidx))[2];
            pointsCols[3*i+0] = _mesh->color(_mesh->vertex_handle(vidx))[0];
            pointsCols[3*i+1] = _mesh->color(_mesh->vertex_handle(vidx))[1];
            pointsCols[3*i+2] = _mesh->color(_mesh->vertex_handle(vidx))[2];
            pointsIndiceArray[i] = i;
            i++;
        }
        vertsSizes.append(qMakePair(vitt.key(), vitt.value().size()));
    }

    ui->displayWidget->loadPoints(pointsVerts, pointsCols, i * 3, pointsIndiceArray, i, vertsSizes);

    delete[] pointsIndiceArray;
    delete[] pointsCols;
    delete[] pointsVerts;
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    edgeSelection = -1;

    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

