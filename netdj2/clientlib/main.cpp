#include <qapplication.h>

#include "NetDJClient.h"
#include "NetLogListener.h"

using namespace NetDJ;

/**
 *
 * @todo Check that interfaces work, that is connect signals to them.
 */
int
main(int argc, char* argv[])
{
  QApplication qapp(argc, argv, false);

  NetDJClient client;
  NetLogListener listener("localhost", 7676);

  QObject::connect(&listener, SIGNAL(SigLogEntry(const QDomElement* aEntry)),
		   &client,   SLOT(NewLogEntry(const QDomElement* aEntry)));

  return 0;
}
