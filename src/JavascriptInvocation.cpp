#include "JavascriptInvocation.h"
#include "WebPage.h"
#include "InvocationResult.h"
#include <QApplication>
#include <QEvent>

JavascriptInvocation::JavascriptInvocation(const QString &functionName, const QStringList &arguments, WebPage *page, QObject *parent) : QObject(parent) {
  m_functionName = functionName;
  m_arguments = arguments;
  m_page = page;
}

QString &JavascriptInvocation::functionName() {
  return m_functionName;
}

QStringList &JavascriptInvocation::arguments() {
  return m_arguments;
}

QVariant JavascriptInvocation::getError() {
  return m_error;
}

void JavascriptInvocation::setError(QVariant error) {
  m_error = error;
}

InvocationResult JavascriptInvocation::invoke(QWebFrame *frame) {
  frame->addToJavaScriptWindowObject("CapybaraInvocation", this);
  QVariant result = frame->evaluateJavaScript("Capybara.invoke()");
  if (getError().isValid())
    return InvocationResult(getError(), true);
  else
    return InvocationResult(result);
}

void JavascriptInvocation::leftClick(int x, int y) {
  QPoint mousePos(x, y);

  JavascriptInvocation::mouseEvent(QEvent::MouseMove, mousePos, Qt::NoButton);
  JavascriptInvocation::mouseEvent(QEvent::MouseButtonPress, mousePos, Qt::LeftButton);
  JavascriptInvocation::mouseEvent(QEvent::MouseButtonRelease, mousePos, Qt::LeftButton);
}

void JavascriptInvocation::rightClick(int x, int y) {
  QPoint mousePos(x, y);

  JavascriptInvocation::mouseEvent(QEvent::MouseMove, mousePos, Qt::NoButton);
  JavascriptInvocation::mouseEvent(QEvent::MouseButtonPress, mousePos, Qt::RightButton);
}

void JavascriptInvocation::doubleClick(int x, int y) {
  QPoint mousePos(x, y);

  JavascriptInvocation::mouseEvent(QEvent::MouseButtonDblClick, mousePos, Qt::LeftButton);
  JavascriptInvocation::mouseEvent(QEvent::MouseButtonRelease, mousePos, Qt::LeftButton);
}

void JavascriptInvocation::mouseEvent(QEvent::Type type, const QPoint &position, Qt::MouseButton button) {
  QMouseEvent event(type, position, button, button, Qt::NoModifier);
  QApplication::sendEvent(m_page, &event);
}

bool JavascriptInvocation::clickTest(QWebElement element, int absoluteX, int absoluteY) {
  QPoint mousePos(absoluteX, absoluteY);
  QWebHitTestResult res = m_page->mainFrame()->hitTestContent(mousePos);
  return res.frame() == element.webFrame();
}

QVariantMap JavascriptInvocation::clickPosition(QWebElement element, int left, int top, int width, int height) {
  QRect elementBox(left, top, width, height);
  QRect viewport(QPoint(0, 0), m_page->viewportSize());
  QRect boundedBox = elementBox.intersected(viewport);
  QPoint mousePos = boundedBox.center();

  QVariantMap m;
  m["relativeX"] = mousePos.x();
  m["relativeY"] = mousePos.y();

  QWebFrame *parent = element.webFrame();
  while (parent) {
    elementBox.translate(parent->geometry().topLeft());
    parent = parent->parentFrame();
  }

  boundedBox = elementBox.intersected(viewport);
  mousePos = boundedBox.center();

  m["absoluteX"] = mousePos.x();
  m["absoluteY"] = mousePos.y();

  return m;
}
