#include "main.h"
#include "MainWindow.h"
#include "ContactsView.h"
#include "HomeContactWidget.h"
#include "ChatWidget.h"
#include "ContactInfoPage.h"
#include "HomeScreen.h"
#include "core/ContactsManager.h"
#include <QToolBar>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QFrame>

MainWindow *uiMain = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	Q_ASSERT(!uiMain);
	uiMain = this;

	setWindowTitle(QString("TorIM"));

	/* Saved geometry */
	resize(config->value("ui/main/windowSize", QSize(730, 400)).toSize());
	QPoint pos = config->value("ui/main/windowPosition").toPoint();
	if (!pos.isNull())
		move(pos);

	/* Center widget */
	QWidget *center = new QWidget;
	setCentralWidget(center);

	QBoxLayout *layout = new QHBoxLayout(center);
	layout->setMargin(0);
	layout->setSpacing(0);

	/* Contacts */
	QBoxLayout *contactsLayout = new QVBoxLayout;
	layout->addLayout(contactsLayout);

	createContactsView();
	contactsLayout->addWidget(contactsView);

	/* Home contact */
	createHomeContact();
	contactsLayout->addWidget(homeContact);

	/* Separator line */
	QFrame *line = new QFrame;
	line->setFrameStyle(QFrame::VLine | QFrame::Plain);

	QPalette p = line->palette();
	p.setColor(QPalette::WindowText, p.color(QPalette::Dark));
	line->setPalette(p);

	layout->addWidget(line);

	/* Chat area */
	createChatArea();
	layout->addWidget(chatArea);

	homeScreen = new HomeScreen;
	chatArea->addWidget(homeScreen);

	showHomeScreen();
}

MainWindow::~MainWindow()
{

}

void MainWindow::createContactsView()
{
	contactsView = new ContactsView;
	contactsView->setFixedWidth(175);

	connect(contactsView, SIGNAL(activePageChanged(ContactUser*,ContactPage)), this,
			SLOT(contactPageChanged(ContactUser*,ContactPage)));
}

void MainWindow::createHomeContact()
{
	homeContact = new HomeContactWidget;
	connect(homeContact, SIGNAL(selected()), this, SLOT(showHomeScreen()));
}

void MainWindow::createChatArea()
{
	chatArea = new QStackedWidget;
	chatArea->setContentsMargins(4, 6, 4, 6);
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
	config->setValue("ui/main/windowSize", size());
	config->setValue("ui/main/windowPosition", pos());

	QMainWindow::closeEvent(ev);
}

void MainWindow::addChatWidget(ChatWidget *widget)
{
	chatArea->addWidget(widget);
}

void MainWindow::showHomeScreen()
{
	if (!homeContact->isSelected())
	{
		homeContact->setSelected(true);
		return;
	}

	contactsView->selectionModel()->clearSelection();
	contactsView->setCurrentIndex(QModelIndex());

	chatArea->setCurrentWidget(homeScreen);
}

void MainWindow::contactPageChanged(ContactUser *user, ContactPage page)
{
	QWidget *old = chatArea->currentWidget();
	QWidget *newWidget = 0;

	homeContact->clearSelected();

	switch (page)
	{
	case ChatPage:
		newWidget = ChatWidget::widgetForUser(user);
		break;
	case InfoPage:
		newWidget = new ContactInfoPage(user);
		break;
	default:
		Q_ASSERT_X(false, "contactPageChanged", "Called for unimplemented page type");
	}

	if (old == newWidget)
		return;

	if (old && !qobject_cast<ChatWidget*>(old) && old != homeScreen)
		old->deleteLater();

	if (newWidget)
		chatArea->setCurrentIndex(chatArea->addWidget(newWidget));
}
