"""
Provides widget classes and functions.

.. warning:: All PyQt4/PySide gui classes are exposed but when you use
    PyQt5, those classes are not available. Therefore, you should treat/use
    this package as if it was ``PyQt5.QtWidgets`` module.
"""
import os
from openalea.plantgl.gui.qt import QT_API, PYQT5_API, PYQT4_API, PYSIDE_API, PYSIDE2_API, PYSIDE6_API

if os.environ[QT_API] in PYSIDE6_API:
    from PySide6.QtWidgets import *
    from PySide6.QtGui import *
elif os.environ[QT_API] in PYQT5_API:
    from PyQt5.QtWidgets import *
elif os.environ[QT_API] in PYQT4_API:
    from PyQt4.QtGui import *
    from PyQt4.QtGui import QFileDialog as OldFileDialog

    class QFileDialog(OldFileDialog):

        @staticmethod
        def getOpenFileName(parent=None, caption='', directory='',
                            filter='', selectedFilter='',
                            options=OldFileDialog.Options()):
            return OldFileDialog.getOpenFileNameAndFilter(
                parent, caption, directory, filter, selectedFilter,
                options)

        @staticmethod
        def getOpenFileNames(parent=None, caption='', directory='',
                             filter='', selectedFilter='',
                             options=OldFileDialog.Options()):
            return OldFileDialog.getOpenFileNamesAndFilter(
                parent, caption, directory, filter, selectedFilter,
                options)

        @staticmethod
        def getSaveFileName(parent=None, caption='', directory='',
                            filter='', selectedFilter='',
                            options=OldFileDialog.Options()):
            return OldFileDialog.getSaveFileNameAndFilter(
                parent, caption, directory, filter, selectedFilter,
                options)
elif os.environ[QT_API] in PYSIDE_API:
    from PySide.QtGui import *
elif os.environ[QT_API] in PYSIDE2_API:
    from PySide2.QtGui import *
