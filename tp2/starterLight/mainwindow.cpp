#include "mainwindow.h"
#include "ui_mainwindow.h"

/* **** début de la partie à compléter **** */
float MainWindow::faceArea(MyMesh *_mesh, int faceID)
{
    /* **** à compléter ! **** */
    FaceHandle face = _mesh->face_handle(faceID);
    int i, j, k, compteur = 0;
    for (MyMesh::FaceVertexIter fv_it = _mesh->fv_iter(face); fv_it.is_valid(); ++fv_it)
    {
        if (compteur == 0)
            i = fv_it->idx();
        else if (compteur == 1)
            j = fv_it->idx();
        else
            k = fv_it->idx();
        compteur = compteur + 1;
    }
    VertexHandle v1 = mesh.vertex_handle(i);
    VertexHandle v2 = mesh.vertex_handle(j);
    VertexHandle v3 = mesh.vertex_handle(k);

    float v1x = _mesh->point(v1)[0];
    float v1y = _mesh->point(v1)[1];
    float v1z = _mesh->point(v1)[2];

    float v2x = _mesh->point(v2)[0];
    float v2y = _mesh->point(v2)[1];
    float v2z = _mesh->point(v2)[2];

    float v3x = _mesh->point(v3)[0];
    float v3y = _mesh->point(v3)[1];
    float v3z = _mesh->point(v3)[2];
    OpenMesh::Vec3f A(v1x, v1y, v1z);
    OpenMesh::Vec3f B(v2x, v2y, v2z);
    OpenMesh::Vec3f C(v3x, v3y, v3z);
    OpenMesh::Vec3f BA = B - A;
    OpenMesh::Vec3f CA = C - A;
    OpenMesh::Vec3f BC = BA % CA;
    double area = sqrt(BC[0] * BC[0] + BC[1] * BC[1] + BC[2] * BC[2]) / 2;
    return area;
    return 0.0;
}

float MainWindow::angleFF(MyMesh *_mesh, int faceID0, int faceID1, int vertID0, int vertID1)
{
    /* **** à compléter ! **** */
    int sign = 0;
    FaceHandle fh0 = _mesh->face_handle(faceID0);
    FaceHandle fh1 = _mesh->face_handle(faceID1);

    MyMesh::FaceVertexCCWIter fvccw = _mesh->fv_ccwiter(fh0);
    if (fvccw->idx() == vertID0)
    {
        fvccw++;
        if (fvccw->idx() == vertID1)
        {
            sign = 1;
        }
        else
        {
            sign = -1;
        }
    }
    OpenMesh::Vec3f normal0(sign * (_mesh->normal(fh0)));
    OpenMesh::Vec3f normal1(_mesh->normal(fh1));

    float scalar = normal0 | normal1;
    return sign * acos(scalar);
}

float MainWindow::angleEE(MyMesh *_mesh, int vertexID, int faceID)
{
    /* **** à compléter ! **** */
    FaceHandle fh = _mesh->face_handle(faceID);
    Vec3f points[3];
    int i = 0;

    for (MyMesh::FaceVertexIter fv_it = _mesh->fv_iter(fh); fv_it.is_valid(); ++fv_it)
    {
        if (fv_it->idx() == vertexID)
        {
            points[0] = _mesh->point(fv_it);
        }
        else
        {
            points[i + 1] = _mesh->point(fv_it);
            i++;
        }
    }

    Vec3f v1 = points[1] - points[0];
    Vec3f v2 = points[2] - points[0];
    float angle = acos(dot(v1, v2) / (norm(v1) * norm(v2)));
    return abs(angle);
}

float MainWindow::calc_area_barycentric(MyMesh *_mesh, int vertexID)
{
    VertexHandle vh = _mesh->vertex_handle(vertexID);
    float area = 0.0;
    for (MyMesh::VertexFaceIter vf_it = _mesh->vf_iter(vh); vf_it.is_valid(); ++vf_it)
    {
        FaceHandle fh = *vf_it;
        area += faceArea(_mesh, fh.idx());
    }
    return (1.0 / 3.0) * area;
}

float MainWindow::calc_sum_angleEE(MyMesh *_mesh, int vertexID)
{
    VertexHandle vh = _mesh->vertex_handle(vertexID);
    float sum = 0.0;

    for (MyMesh::VertexFaceIter vf_it = _mesh->vf_iter(vh); vf_it.is_valid(); ++vf_it)
    {
        FaceHandle fh = *vf_it;
        sum += angleEE(_mesh, vf_it->idx(), fh.idx());
    }

    return sum;
}

float MainWindow::calc_sum_angleFF(MyMesh *_mesh, int vertexID)
{
    VertexHandle vh = _mesh->vertex_handle(vertexID);
    float sum = 0.0;
    for (MyMesh::VertexOHalfedgeIter voh_it = _mesh->voh_iter(vh); voh_it.is_valid(); ++voh_it)
    {
        HalfedgeHandle heh = *voh_it;
        FaceHandle fh0 = _mesh->face_handle(heh);
        if(!fh0.is_valid()) continue;
        FaceHandle fh1 = _mesh->opposite_face_handle(heh);
        if(!fh1.is_valid()) continue;

        VertexHandle vh0 = _mesh->from_vertex_handle(heh);
        VertexHandle vh1 = _mesh->to_vertex_handle(heh);

        float length = (_mesh->point(vh1)-_mesh->point(vh0)).norm();
        sum += angleFF(_mesh,fh0.idx(),fh1.idx(),vh0.idx(), vh1.idx()) * length;
    }

    return sum;
}

void MainWindow::H_Curv(MyMesh *_mesh)
{
    /* **** à compléter ! **** */
    for (MyMesh::VertexIter curVert = _mesh->vertices_begin(); curVert != _mesh->vertices_end(); ++curVert)
    {
        VertexHandle vh = *curVert;
        float part1 = 1.0 / (4.0* calc_area_barycentric(_mesh, vh.idx()));
        float part2 = calc_sum_angleFF(_mesh, vh.idx());

        _mesh->data(vh).value = part1 * part2 ;
    }
}

void MainWindow::K_Curv(MyMesh *_mesh)
{
    /* **** à compléter ! **** */
     for(MyMesh::VertexIter curVert = _mesh->vertices_begin(); curVert!=_mesh->vertices_end();++curVert){
        VertexHandle vh = *curVert;
        float part1 = 1.0/calc_area_barycentric(_mesh,vh.idx());
        float part2 = 2*M_PI - calc_sum_angleEE(_mesh,vh.idx());
        _mesh->data(vh).value = part1 * part2;
    }
}
/* **** fin de la partie à compléter **** */

/* **** début de la partie boutons et IHM **** */
void MainWindow::on_pushButton_H_clicked()
{
    H_Curv(&mesh);
    displayMesh(&mesh, DisplayMode::TemperatureMap); // permet de passer en mode "carte de temperatures", avec une gestion automatique de la couleur (voir exemple)
}

void MainWindow::on_pushButton_K_clicked()
{
    K_Curv(&mesh);
    displayMesh(&mesh, DisplayMode::TemperatureMap); // permet de passer en mode "carte de temperatures", avec une gestion automatique de la couleur (voir exemple)
}

/*  Cette fonction est à utiliser UNIQUEMENT avec le fichier testAngleArea.obj
    Elle est appelée par le bouton "Test angles/aires"

    Elle permet de vérifier les fonctions faceArea, angleFF et angleEE.
    Elle doit afficher :

    Aire de la face 0 : 2
    Aire de la face 1 : 2
    Angle entre les faces 0 et 1 : 1.5708
    Angle entre les faces 1 et 0 : -1.5708
    Angle au sommet 1 sur la face 0 : 0.785398 */
void MainWindow::on_pushButton_angleArea_clicked()
{
    qDebug() << "Aire de la face 0 :" << faceArea(&mesh, 0);
    qDebug() << "Aire de la face 1 :" << faceArea(&mesh, 1);

    qDebug() << "Angle entre les faces 0 et 1 :" << angleFF(&mesh, 0, 1, 1, 2);
    qDebug() << "Angle entre les faces 1 et 0 :" << angleFF(&mesh, 1, 0, 1, 2);

    qDebug() << "Angle au sommet 1 sur la face 0 :" << angleEE(&mesh, 1, 0);
    qDebug() << "Angle au sommet 3 sur la face 1 :" << angleEE(&mesh, 3, 1);
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
/* **** fin de la partie boutons et IHM **** */

/* **** fonctions supplémentaires **** */
// permet d'initialiser les couleurs et les épaisseurs des élements du maillage
void MainWindow::resetAllColorsAndThickness(MyMesh *_mesh)
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
void MainWindow::displayMesh(MyMesh *_mesh, DisplayMode mode)
{
    GLuint *triIndiceArray = new GLuint[_mesh->n_faces() * 3];
    GLfloat *triCols = new GLfloat[_mesh->n_faces() * 3 * 3];
    GLfloat *triVerts = new GLfloat[_mesh->n_faces() * 3 * 3];

    int i = 0;

    if (mode == DisplayMode::TemperatureMap)
    {
        QVector<float> values;
        for (MyMesh::VertexIter curVert = _mesh->vertices_begin(); curVert != _mesh->vertices_end(); curVert++)
            values.append(fabs(_mesh->data(*curVert).value));
        std::sort(values.begin(), values.end());

        float range = values.at(values.size() * 0.8);

        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;

        for (; fIt != fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            if (_mesh->data(*fvIt).value > 0)
            {
                triCols[3 * i + 0] = 255;
                triCols[3 * i + 1] = 255 - std::min((_mesh->data(*fvIt).value / range) * 255.0, 255.0);
                triCols[3 * i + 2] = 255 - std::min((_mesh->data(*fvIt).value / range) * 255.0, 255.0);
            }
            else
            {
                triCols[3 * i + 2] = 255;
                triCols[3 * i + 1] = 255 - std::min((-_mesh->data(*fvIt).value / range) * 255.0, 255.0);
                triCols[3 * i + 0] = 255 - std::min((-_mesh->data(*fvIt).value / range) * 255.0, 255.0);
            }
            triVerts[3 * i + 0] = _mesh->point(*fvIt)[0];
            triVerts[3 * i + 1] = _mesh->point(*fvIt)[1];
            triVerts[3 * i + 2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
            ++fvIt;
            if (_mesh->data(*fvIt).value > 0)
            {
                triCols[3 * i + 0] = 255;
                triCols[3 * i + 1] = 255 - std::min((_mesh->data(*fvIt).value / range) * 255.0, 255.0);
                triCols[3 * i + 2] = 255 - std::min((_mesh->data(*fvIt).value / range) * 255.0, 255.0);
            }
            else
            {
                triCols[3 * i + 2] = 255;
                triCols[3 * i + 1] = 255 - std::min((-_mesh->data(*fvIt).value / range) * 255.0, 255.0);
                triCols[3 * i + 0] = 255 - std::min((-_mesh->data(*fvIt).value / range) * 255.0, 255.0);
            }
            triVerts[3 * i + 0] = _mesh->point(*fvIt)[0];
            triVerts[3 * i + 1] = _mesh->point(*fvIt)[1];
            triVerts[3 * i + 2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
            ++fvIt;
            if (_mesh->data(*fvIt).value > 0)
            {
                triCols[3 * i + 0] = 255;
                triCols[3 * i + 1] = 255 - std::min((_mesh->data(*fvIt).value / range) * 255.0, 255.0);
                triCols[3 * i + 2] = 255 - std::min((_mesh->data(*fvIt).value / range) * 255.0, 255.0);
            }
            else
            {
                triCols[3 * i + 2] = 255;
                triCols[3 * i + 1] = 255 - std::min((-_mesh->data(*fvIt).value / range) * 255.0, 255.0);
                triCols[3 * i + 0] = 255 - std::min((-_mesh->data(*fvIt).value / range) * 255.0, 255.0);
            }
            triVerts[3 * i + 0] = _mesh->point(*fvIt)[0];
            triVerts[3 * i + 1] = _mesh->point(*fvIt)[1];
            triVerts[3 * i + 2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }

    if (mode == DisplayMode::Normal)
    {
        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;
        for (; fIt != fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            triCols[3 * i + 0] = _mesh->color(*fIt)[0];
            triCols[3 * i + 1] = _mesh->color(*fIt)[1];
            triCols[3 * i + 2] = _mesh->color(*fIt)[2];
            triVerts[3 * i + 0] = _mesh->point(*fvIt)[0];
            triVerts[3 * i + 1] = _mesh->point(*fvIt)[1];
            triVerts[3 * i + 2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
            ++fvIt;
            triCols[3 * i + 0] = _mesh->color(*fIt)[0];
            triCols[3 * i + 1] = _mesh->color(*fIt)[1];
            triCols[3 * i + 2] = _mesh->color(*fIt)[2];
            triVerts[3 * i + 0] = _mesh->point(*fvIt)[0];
            triVerts[3 * i + 1] = _mesh->point(*fvIt)[1];
            triVerts[3 * i + 2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
            ++fvIt;
            triCols[3 * i + 0] = _mesh->color(*fIt)[0];
            triCols[3 * i + 1] = _mesh->color(*fIt)[1];
            triCols[3 * i + 2] = _mesh->color(*fIt)[2];
            triVerts[3 * i + 0] = _mesh->point(*fvIt)[0];
            triVerts[3 * i + 1] = _mesh->point(*fvIt)[1];
            triVerts[3 * i + 2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }

    if (mode == DisplayMode::ColorShading)
    {
        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;
        for (; fIt != fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            triCols[3 * i + 0] = _mesh->data(*fvIt).faceShadingColor[0];
            triCols[3 * i + 1] = _mesh->data(*fvIt).faceShadingColor[1];
            triCols[3 * i + 2] = _mesh->data(*fvIt).faceShadingColor[2];
            triVerts[3 * i + 0] = _mesh->point(*fvIt)[0];
            triVerts[3 * i + 1] = _mesh->point(*fvIt)[1];
            triVerts[3 * i + 2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
            ++fvIt;
            triCols[3 * i + 0] = _mesh->data(*fvIt).faceShadingColor[0];
            triCols[3 * i + 1] = _mesh->data(*fvIt).faceShadingColor[1];
            triCols[3 * i + 2] = _mesh->data(*fvIt).faceShadingColor[2];
            triVerts[3 * i + 0] = _mesh->point(*fvIt)[0];
            triVerts[3 * i + 1] = _mesh->point(*fvIt)[1];
            triVerts[3 * i + 2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
            ++fvIt;
            triCols[3 * i + 0] = _mesh->data(*fvIt).faceShadingColor[0];
            triCols[3 * i + 1] = _mesh->data(*fvIt).faceShadingColor[1];
            triCols[3 * i + 2] = _mesh->data(*fvIt).faceShadingColor[2];
            triVerts[3 * i + 0] = _mesh->point(*fvIt)[0];
            triVerts[3 * i + 1] = _mesh->point(*fvIt)[1];
            triVerts[3 * i + 2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }

    ui->displayWidget->loadMesh(triVerts, triCols, _mesh->n_faces() * 3 * 3, triIndiceArray, _mesh->n_faces() * 3);

    delete[] triIndiceArray;
    delete[] triCols;
    delete[] triVerts;

    GLuint *linesIndiceArray = new GLuint[_mesh->n_edges() * 2];
    GLfloat *linesCols = new GLfloat[_mesh->n_edges() * 2 * 3];
    GLfloat *linesVerts = new GLfloat[_mesh->n_edges() * 2 * 3];

    i = 0;
    QHash<float, QList<int>> edgesIDbyThickness;
    for (MyMesh::EdgeIter eit = _mesh->edges_begin(); eit != _mesh->edges_end(); ++eit)
    {
        float t = _mesh->data(*eit).thickness;
        if (t > 0)
        {
            if (!edgesIDbyThickness.contains(t))
                edgesIDbyThickness[t] = QList<int>();
            edgesIDbyThickness[t].append((*eit).idx());
        }
    }
    QHashIterator<float, QList<int>> it(edgesIDbyThickness);
    QList<QPair<float, int>> edgeSizes;
    while (it.hasNext())
    {
        it.next();

        for (int e = 0; e < it.value().size(); e++)
        {
            int eidx = it.value().at(e);

            MyMesh::VertexHandle vh1 = _mesh->to_vertex_handle(_mesh->halfedge_handle(_mesh->edge_handle(eidx), 0));
            linesVerts[3 * i + 0] = _mesh->point(vh1)[0];
            linesVerts[3 * i + 1] = _mesh->point(vh1)[1];
            linesVerts[3 * i + 2] = _mesh->point(vh1)[2];
            linesCols[3 * i + 0] = _mesh->color(_mesh->edge_handle(eidx))[0];
            linesCols[3 * i + 1] = _mesh->color(_mesh->edge_handle(eidx))[1];
            linesCols[3 * i + 2] = _mesh->color(_mesh->edge_handle(eidx))[2];
            linesIndiceArray[i] = i;
            i++;

            MyMesh::VertexHandle vh2 = _mesh->from_vertex_handle(_mesh->halfedge_handle(_mesh->edge_handle(eidx), 0));
            linesVerts[3 * i + 0] = _mesh->point(vh2)[0];
            linesVerts[3 * i + 1] = _mesh->point(vh2)[1];
            linesVerts[3 * i + 2] = _mesh->point(vh2)[2];
            linesCols[3 * i + 0] = _mesh->color(_mesh->edge_handle(eidx))[0];
            linesCols[3 * i + 1] = _mesh->color(_mesh->edge_handle(eidx))[1];
            linesCols[3 * i + 2] = _mesh->color(_mesh->edge_handle(eidx))[2];
            linesIndiceArray[i] = i;
            i++;
        }
        edgeSizes.append(qMakePair(it.key(), it.value().size()));
    }

    ui->displayWidget->loadLines(linesVerts, linesCols, i * 3, linesIndiceArray, i, edgeSizes);

    delete[] linesIndiceArray;
    delete[] linesCols;
    delete[] linesVerts;

    GLuint *pointsIndiceArray = new GLuint[_mesh->n_vertices()];
    GLfloat *pointsCols = new GLfloat[_mesh->n_vertices() * 3];
    GLfloat *pointsVerts = new GLfloat[_mesh->n_vertices() * 3];

    i = 0;
    QHash<float, QList<int>> vertsIDbyThickness;
    for (MyMesh::VertexIter vit = _mesh->vertices_begin(); vit != _mesh->vertices_end(); ++vit)
    {
        float t = _mesh->data(*vit).thickness;
        if (t > 0)
        {
            if (!vertsIDbyThickness.contains(t))
                vertsIDbyThickness[t] = QList<int>();
            vertsIDbyThickness[t].append((*vit).idx());
        }
    }
    QHashIterator<float, QList<int>> vitt(vertsIDbyThickness);
    QList<QPair<float, int>> vertsSizes;

    while (vitt.hasNext())
    {
        vitt.next();

        for (int v = 0; v < vitt.value().size(); v++)
        {
            int vidx = vitt.value().at(v);

            pointsVerts[3 * i + 0] = _mesh->point(_mesh->vertex_handle(vidx))[0];
            pointsVerts[3 * i + 1] = _mesh->point(_mesh->vertex_handle(vidx))[1];
            pointsVerts[3 * i + 2] = _mesh->point(_mesh->vertex_handle(vidx))[2];
            pointsCols[3 * i + 0] = _mesh->color(_mesh->vertex_handle(vidx))[0];
            pointsCols[3 * i + 1] = _mesh->color(_mesh->vertex_handle(vidx))[1];
            pointsCols[3 * i + 2] = _mesh->color(_mesh->vertex_handle(vidx))[2];
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
    vertexSelection = -1;
    edgeSelection = -1;
    faceSelection = -1;

    modevoisinage = false;

    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
