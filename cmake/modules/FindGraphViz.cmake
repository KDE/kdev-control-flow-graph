###########################################################################
#   Copyright 2009 Sandro Andrade <sandroandrade@kde.org>                 #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU Library General Public License as       #
#   published by the Free Software Foundation; either version 2 of the    #
#   License, or (at your option) any later version.                       #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU Library General Public     #
#   License along with this program; if not, write to the                 #
#   Free Software Foundation, Inc.,                                       #
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         #
###########################################################################/

if (NOT WIN32)
  find_package(PkgConfig)
  pkg_check_modules(graphviz ${REQUIRED} libgvc libcdt libcgraph libpathplan)
  if (GraphViz_FOUND)
    set (GraphViz_INCLUDE_DIRECTORIES ${GraphViz_INCLUDE_DIRS})
  endif (GraphViz_FOUND)
endif (NOT WIN32)
    
find_path(GraphViz_INCLUDE_DIRECTORIES
  NAMES gvc.h
  PATHS
  ${GraphViz_INCLUDE_DIRS}
  /usr/local/include/graphviz
  /usr/include/graphviz
)
    
find_library(GraphViz_GVC_LIBRARY
  NAMES gvc
  PATHS
  ${GraphViz_LIBRARY_DIRS}
)

find_library(GraphViz_CDT_LIBRARY
  NAMES cdt
  PATHS
  ${GraphViz_LIBRARY_DIRS}
)

find_library(GraphViz_GRAPH_LIBRARY
  NAMES cgraph
  PATHS
  ${GraphViz_LIBRARY_DIRS}
)

find_library(GraphViz_PATHPLAN_LIBRARY
  NAMES pathplan
  PATHS
  ${GraphViz_LIBRARY_DIRS}
)

if (GraphViz_INCLUDE_DIRECTORIES AND
    GraphViz_GVC_LIBRARY AND GraphViz_CDT_LIBRARY AND
    GraphViz_GRAPH_LIBRARY AND GraphViz_PATHPLAN_LIBRARY)
  set (GraphViz_FOUND 1)
  set (GraphViz_LIBRARIES
       "${GraphViz_GVC_LIBRARY};${GraphViz_GRAPH_LIBRARY};"
       "${GraphViz_CDT_LIBRARY};${GraphViz_PATHPLAN_LIBRARY}"
       CACHE FILEPATH "Libraries for graphviz")
else (GraphViz_INCLUDE_DIRECTORIES AND
      GraphViz_GVC_LIBRARY AND GraphViz_CDT_LIBRARY AND
      GraphViz_GRAPH_LIBRARY AND GraphViz_PATHPLAN_LIBRARY)
  set (GraphViz_FOUND 0)
  if (GraphViz_FIND_REQUIRED)
    message (FATAL_ERROR "GraphViz not installed !")
  endif (GraphViz_FIND_REQUIRED)
endif (GraphViz_INCLUDE_DIRECTORIES AND
GraphViz_GVC_LIBRARY AND GraphViz_CDT_LIBRARY AND
GraphViz_GRAPH_LIBRARY AND GraphViz_PATHPLAN_LIBRARY)
