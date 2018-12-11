#ifndef VIEW_H
#define VIEW_H

#include <QOpenGLWidget>
#include "myutils.hpp"
#include "d_format.hpp"

struct ViewData;

class View : public QOpenGLWidget
{
    Q_OBJECT

private:
    ViewData *viewData;
    QString status;

    friend struct ViewData;

public:
    View(QWidget *parent);
    virtual ~View();

    void initializeGL() override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
private:
    void updateZoom();
    void onKeyEvent(bool press, QKeyEvent *event);
    void updateStatusString(const QString &str);
signals:
    void statusChanged(QString status);
    void keyEvent(const Obj &obj);

public slots:
    void changeFrame(std::shared_ptr<Frame> renderFrame);
    void fieldSizeChange(int w, int h);
    void field3d(const P &minP, const P &maxP, double hMin, double hMax, double cellSize);
    void addStaticObject(const SObj &sobj);
    void settingsChanged();

private:
    std::shared_ptr<Frame> renderFrame;

    int w = 1000, h = 1000;
};

#endif // VIEW_H
