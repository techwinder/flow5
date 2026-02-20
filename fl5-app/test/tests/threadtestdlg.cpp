/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


*****************************************************************************/


// Visual studio bug override
//https://developercommunity.visualstudio.com/t/Visual-Studio-17100-Update-leads-to-Pr/10669759?sort=newest
//#define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <QCoreApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include "threadtestdlg.h"
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>

#include <api/constants.h>
#include <api/matrix.h>
#include <api/flow5events.h>
#include <api/fl5color.h>
#include <math/testmatrix.h>

QByteArray ThreadTestDlg::s_Geometry;


ThreadTestDlg::ThreadTestDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
    setMinimumSize(700,700);
}


void ThreadTestDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_ppto = new PlainTextOutput;

        QHBoxLayout *pOptionsLayout = new QHBoxLayout;
        {
            QLabel *plabNThreads = new QLabel("Nbr. of threads=");
            m_pieNThreads = new IntEdit(QThread::idealThreadCount());

            pOptionsLayout->addWidget(plabNThreads);
            pOptionsLayout->addWidget(m_pieNThreads);
            pOptionsLayout->addStretch();
        }


        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            m_ppbMulti = new QPushButton("Test multitask");
            m_ppbComm  = new QPushButton("Test communication");

            m_pButtonBox->addButton(m_ppbMulti, QDialogButtonBox::ActionRole);
            m_pButtonBox->addButton(m_ppbComm, QDialogButtonBox::ActionRole);

            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addLayout(pOptionsLayout);
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void ThreadTestDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)  reject();
    else if (m_ppbMulti == pButton)  testMulti();
    else if (m_ppbComm  == pButton)  testComm();
}


void ThreadTestDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const*pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendQText("Received event: " + pMsgEvent->msg()+"\n");
    }
    else
        QDialog::customEvent(pEvent);
}


void ThreadTestDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void ThreadTestDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void ThreadTestDlg::testMulti()
{
    int nThreads = m_pieNThreads->value();
    m_ppto->onAppendQText(QString::asprintf("Using %d threads\n\n", nThreads));

    int m = 123;
    int n = 237;
    int q = 319;

    std::vector<double> A(m*n, 0);

    for(int i=0; i<m; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(5.1+(2.0*double(i)-double(j))*double(i-j+1));

    std::vector<double> B(n*q, 0);

    for(int i=0; i<n; i++)
        for(int j=0; j<q; j++)
            B[i*q+j]=1.0+double(i-j);

    std::vector<double> AB(m*q, 0);

    auto t0 = std::chrono::high_resolution_clock::now();

    matrix::matMult_SingleThread(A.data(),B.data(), AB.data(), m, n, q);

    auto t1 = std::chrono::high_resolution_clock::now();
    int durationSingle = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    double checksumSingle = AB[35] + AB[111] + AB[151] + AB[712] + AB[1351];

    std::fill(AB.begin(), AB.end(), 0.0);
    matrix::matMult(A.data(),B.data(), AB.data(), m, n, q, nThreads);
    double checksumStd = AB[35] + AB[111] + AB[151] + AB[712] + AB[1351];


    auto t2 = std::chrono::high_resolution_clock::now();
    int durationStd = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();


    QString strange = QString::asprintf("MatMult times: Single: %g ms  Std(%d): %g ms \n",
                                        double(durationSingle)/1000.0,  nThreads, double(durationStd)/1000.0);

    m_ppto->onAppendQText(strange);


    m_ppto->onAppendQText(QString::asprintf("Checksums= %g   %g\n\n", checksumSingle, checksumStd));
}


#define MAX_PROD 20



std::mutex mtx;
std::condition_variable cond_var;
std::queue<Product> theQueue;

void ThreadTestDlg::producer()
{
    Product produced;

    while (produced.a<MAX_PROD)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));   // simulate some pause between productions
        produced.a++;
        produced.b = cos(1.7964394*double(produced.a));
        produced.txt = QString::asprintf("prod[%d]. = %g", produced.a, produced.b).toStdString();

        // Access the Q under the lock:
        std::unique_lock<std::mutex> lck(mtx);
        theQueue.push(produced);
        cond_var.notify_all();

    }
    std::cout << "done producing" << std::endl;
}


void ThreadTestDlg::consumer()
{
    while (m_Prod.a<MAX_PROD)
    {
        // Access the Q under the lock:
        std::unique_lock<std::mutex> lck(mtx);
        // NOTE: The following call will lock the mutex only when the condition_variable will cause wakeup
        //       (due to `notify` or spurious wakeup).
        //       Then it will check if the Q is empty.
        //       If empty it will release the lock and continue to wait.
        //       If not empty, the lock will be kept until out of scope.
        //       See the documentation for std::condition_variable.
        cond_var.wait(lck, []() { return !theQueue.empty(); }); // will loop internally to handle spurious wakeups
        m_Prod = theQueue.front();
        theQueue.pop();

//        std::cout << "consumed: " << m_Prod.txt << std::endl;

        MessageEvent *pMsgEvent = new MessageEvent(m_Prod.txt); // forward to the UI thread for user notification
        qApp->postEvent(this, pMsgEvent);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));    // simulate some calculation
    }
//    std::cout << "exiting consumer" << std::endl;


    MessageEvent *pMsgEvent = new MessageEvent(QString("Done consuming\n"));
    qApp->postEvent(this, pMsgEvent);
}


void ThreadTestDlg::testComm()
{
    //running this function in a separate thread to keep the UI responsive
    QtConcurrent::run([this](){ this->runComm(); });


}

void ThreadTestDlg::runComm()
{
    std::thread thethread(&ThreadTestDlg::producer, this);
//    std::thread c(&ThreadTestDlg::consumer, this); // no need - consume in this thread instead

    MessageEvent *pMsgEvent = new MessageEvent(QString("All threads started\n")); // notify the UI thread
    qApp->postEvent(this, pMsgEvent);

    while(m_Prod.a<MAX_PROD)
    {
        // Access the Q under the lock:
        std::unique_lock<std::mutex> lck(mtx);
        cond_var.wait(lck, []() { return !theQueue.empty(); }); // will loop internally to handle spurious wakeups
        m_Prod = theQueue.front();
        theQueue.pop();

        MessageEvent *pMsgEvent = new MessageEvent(m_Prod.txt); // forward to the UI thread for user notification
        qApp->postEvent(this, pMsgEvent);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));    // simulate some calculation
    }

    pMsgEvent = new MessageEvent(QString("Joining threads....\n"));
    qApp->postEvent(this, pMsgEvent);

    thethread.join();
//    c.join();

    pMsgEvent = new MessageEvent(QString(" --- > joined\n"));
    qApp->postEvent(this, pMsgEvent);
}





