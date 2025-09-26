/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#include <QOpenGLWidget>
#include <QDialog>
#include <QRadioButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QSurface>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QSplitter>
#include <QMessageBox>
#include <QSettings>

class PlainTextOutput;
class gl3dTestGLView;
class IntEdit;

class OpenGlDlg : public QDialog
{
    Q_OBJECT
    public:
        OpenGlDlg(QWidget *pParent=nullptr);

        void initDialog();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onCreateContext();
        void onRenderWindowReady();
        void onRenderWindowError(const QString &msg);
        void onSettingsChanged();
        void onMultiSampling();
        void onButton(QAbstractButton *pButton);
        void onApply();
        void reject() override;
        void onViewType();


    private:
        QSize sizeHint() const override {return QSize(1200,1500);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

        void readFormat(QSurfaceFormat &fmt);
        void readVersion(QPair<int, int> &oglversion);

        QOpenGLWidget *getView(int iView);

        void printFormat(const QSurfaceFormat &format, QString &log, bool bFull=true);
        void setupLayout();

        void enableControls(QPair<int, int> oglversion);
        QMessageBox::StandardButton applyChanges();

//-----------Variables ----------------

        QPushButton *m_ppbApply;
        QPushButton *m_ppbTestView;

        QDialogButtonBox *m_pButtonBox;
        QComboBox *m_pcbVersion;
        QCheckBox *m_pchDeprecatedFcts;
        QCheckBox *m_pchMultiSampling;
        PlainTextOutput *m_pptglOutput;

        QLabel *m_plabMemStatus;

        IntEdit *m_pieSamples;

        QRadioButton *m_prbProfiles[3];

        QStackedWidget *m_pStackWt;

        QOpenGLWidget *m_pglTestView;

        QSplitter *m_pHSplitter;

        bool m_bChanged;

        QSurfaceFormat m_SavedFormat;

        static int s_iView;

        static QByteArray s_HSplitterSizes;
        static QByteArray s_Geometry;
};

