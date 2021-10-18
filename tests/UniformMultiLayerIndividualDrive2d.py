import unittest

import FrictionQPotFEM
import GMatElastoPlasticQPot.Cartesian2d as GMat
import GooseFEM
import numpy as np


class test_Generic2d(unittest.TestCase):
    """
    Tests
    """

    def test_eventDrivenSimpleShear(self):

        mesh = GooseFEM.Mesh.Quad4.Regular(nx=3, ny=2 * 3 + 1)
        nelem = mesh.nelem()
        conn = mesh.conn()
        elem = mesh.elementgrid()
        dofs = mesh.dofs()
        dofs[mesh.nodesLeftEdge()[1:], ...] = dofs[mesh.nodesRightEdge()[1:], ...]
        layers = [elem[:3, :].ravel(), elem[3, :].ravel(), elem[4:, :].ravel()]

        system = FrictionQPotFEM.UniformMultiLayerIndividualDrive2d.System(
            mesh.coor(),
            mesh.conn(),
            dofs,
            dofs[mesh.nodesBottomEdge(), :].ravel(),
            layers,
            [np.unique(conn[i, :]) for i in layers],
            [False, True, False],
        )

        nelas = system.elastic().size
        nplas = system.plastic().size

        epsy = 1e-3 * np.cumsum(np.ones((nplas, 5)), axis=1)

        system.setMassMatrix(np.ones(nelem))
        system.setDampingMatrix(np.ones(nelem))
        system.setElastic(np.ones(nelas), np.ones(nelas))
        system.setPlastic(np.ones(nplas), np.ones(nplas), epsy)
        system.setDt(1.0)
        system.layerSetDriveStiffness(1e-3)

        drive_active = np.zeros((3, 2), dtype=bool)
        drive_u = np.zeros((3, 2), dtype=float)

        drive_active[-1, 0] = True
        drive_u[-1, 0] = 0.1

        system.initEventDriven(drive_u, drive_active)

        system.eventDrivenStep(1e-4, False)
        self.assertTrue(np.allclose(GMat.Epsd(system.plastic_Eps()), epsy[0, 0] - 0.5 * 1e-4))

        system.eventDrivenStep(1e-4, False)
        self.assertTrue(np.allclose(GMat.Epsd(system.plastic_Eps()), epsy[0, 0] - 0.5 * 1e-4))

        system.eventDrivenStep(1e-4, True)
        self.assertTrue(np.allclose(GMat.Epsd(system.plastic_Eps()), epsy[0, 0] + 0.5 * 1e-4))

        system.eventDrivenStep(1e-4, False)
        self.assertTrue(np.allclose(GMat.Epsd(system.plastic_Eps()), epsy[0, 1] - 0.5 * 1e-4))

        system.eventDrivenStep(1e-4, True)
        self.assertTrue(np.allclose(GMat.Epsd(system.plastic_Eps()), epsy[0, 1] + 0.5 * 1e-4))

        system.eventDrivenStep(1e-4, False)
        self.assertTrue(np.allclose(GMat.Epsd(system.plastic_Eps()), epsy[0, 2] - 0.5 * 1e-4))

        system.eventDrivenStep(1e-4, False, -1)
        self.assertTrue(np.allclose(GMat.Epsd(system.plastic_Eps()), epsy[0, 1] + 0.5 * 1e-4))

        system.eventDrivenStep(1e-4, True, -1)
        self.assertTrue(np.allclose(GMat.Epsd(system.plastic_Eps()), epsy[0, 1] - 0.5 * 1e-4))

        system.eventDrivenStep(1e-4, False, -1)
        self.assertTrue(np.allclose(GMat.Epsd(system.plastic_Eps()), epsy[0, 0] + 0.5 * 1e-4))


if __name__ == "__main__":

    unittest.main()