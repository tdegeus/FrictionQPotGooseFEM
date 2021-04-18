import numpy as np
import GooseFEM as gf
import FrictionQPotFEM.UniformMultiLayerIndividualDrive2d as model

layer_elas = gf.Mesh.Quad4.Regular(20, 6)
layer_plas = gf.Mesh.Quad4.Regular(20, 1)

stitch = gf.Mesh.Stitch()

x0 = layer_elas.coor()
x1 = layer_plas.coor()
x2 = layer_elas.coor()

x1[:, 1] += np.max(x0[:, 1])
x2[:, 1] += np.max(x1[:, 1])

stitch.push_back(x0, layer_elas.conn())
stitch.push_back(x1, layer_plas.conn())
stitch.push_back(x2, layer_elas.conn())

left = stitch.nodeset([
    layer_elas.nodesLeftOpenEdge(),
    layer_plas.nodesLeftEdge(),
    layer_elas.nodesLeftOpenEdge(),
])

right = stitch.nodeset([
    layer_elas.nodesRightOpenEdge(),
    layer_plas.nodesRightEdge(),
    layer_elas.nodesRightOpenEdge(),
])

bottom = stitch.nodeset(layer_elas.nodesBottomEdge(), 0)
top = stitch.nodeset(layer_elas.nodesTopEdge(), 2)

nelem = stitch.nelem()
coor = stitch.coor()
conn = stitch.conn()

dofs = stitch.dofs()
dofs[right, :] = dofs[left, :]
dofs[top[0], 0] = dofs[top[-1], 0]
dofs[bottom[0], 0] = dofs[bottom[-1], 0]
dofs = gf.Mesh.renumber(dofs)

iip = np.concatenate((dofs[bottom, :].ravel(), dofs[top, 1].ravel()))

elas = list(stitch.elemmap(0)) + list(stitch.elemmap(2))
plas = list(stitch.elemmap(1))

epsy = np.cumsum(0.01 * np.ones((len(plas), 100)), axis=1)

system = model.System(coor, conn, dofs, iip, stitch.elemmap(), stitch.nodemap(), [False, True, False])
system.setMassMatrix(1.0 * np.ones((nelem)))
system.setDampingMatrix(0.01 * np.ones((nelem)))
system.setElastic(10.0 * np.ones((len(elas))), 1.0 * np.ones((len(elas))))
system.setPlastic(10.0 * np.ones((len(plas))), 1.0 * np.ones((len(plas))), epsy)
system.setDt(0.1)
system.setDriveStiffness(1.0)

ubar = np.zeros((3, 2))
drive = np.zeros((3, 2), dtype=bool)
ubar[2, 0] = 0.5
drive[2, 0] = True

system.layerSetUbar(ubar, drive)

system.minimise()

try:

    c = np.zeros((stitch.nelem()))
    for i in range(3):
        c[stitch.elemmap(i)] = i

    import matplotlib.pyplot as plt
    import GooseMPL as gplt

    plt.style.use(['goose', 'goose-latex'])

    fig, ax = plt.subplots()

    x = coor + system.u()
    gplt.patch(coor=x, conn=conn, cindex=c)
    plt.show()

except:

    pass

