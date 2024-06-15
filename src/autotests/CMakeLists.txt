# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata" )

ecm_add_test(navigationcontrollertest.cpp
    TEST_NAME navigationcontroller
    LINK_LIBRARIES marknote_static Qt::Test
    NAME_PREFIX "marknote-"
)

ecm_add_test(notebooksmodeltest.cpp
    TEST_NAME notebooksmodel
    LINK_LIBRARIES marknote_static Qt::Test
    NAME_PREFIX "marknote-"
)

ecm_add_test(nestedlisthelpertest.cpp
    TEST_NAME nestedlisthelper
    LINK_LIBRARIES marknote_static Qt::Test
    NAME_PREFIX "marknote-"
)

ecm_add_test(maildirimporttest.cpp
    TEST_NAME maildirimporttest
    LINK_LIBRARIES marknote_static Qt::Test
    NAME_PREFIX "marknote-"
)
